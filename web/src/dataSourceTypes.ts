import { cStruct, uint8, type CTypeBase, type CType, float, uint16 } from "c-type-util";

export let dataSources = {
    "unsigned char": uint8,
    "sin_waves_t": cStruct({
        s5: float,
        s10: float,
        s1: float,
    }),
    "short unsigned int": uint16,
}



export let expDataSources = expandSourceDefinitions(dataSources);

export function getSourceType(typeName: string): CType<any> {
    if(expDataSources[typeName] == null)
        throw new Error(`Data source definition for type name >${typeName}< not found.`);
    return expDataSources[typeName];
}

/**
 * "Expands" the input "generic" typename-type map to include implementation-specific
 * "actual" typenames. For a type "char", for example, it will make the source include
 * an entry like "constexpr const char* t() [with T = char]" to match GCC's
 * __PRETTY_FUNCTION__, which is used to generate a characteristic type name.
 * See the firmware for more information.
 * @param sources 
 * @returns 
 */
function expandSourceDefinitions(sources: { [key: string]: CTypeBase<any> }) {
    let mapped = {};
    for (let k in sources) {
        mapped[k] = sources[k];
        mapped[`constexpr const char* t() [with T = ${k}]`] = sources[k];
    }

    return mapped;
}