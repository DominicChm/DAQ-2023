#pragma once
#include <vector>

#include "../dlf_util.h"

/**
 * @brief Base class for components (plugins) 
 * 
 * This class should be inherited by all plugins. It provides lifecycle methods used to manage
 * plugins externally (ie begin), and defines the API plugins can use (under protected)
 * 
 * Plugins are singletons, so can be looked up by their type.
 */
class BaseComponent {
   private:
    // Pointer to a common store of all components. Populated when the plugin is instantiated by the caller.
    std::vector<BaseComponent *> *component_store;
    size_t id;

   public:
    void setup(std::vector<BaseComponent *> *store) {
        component_store = store;
    }

    /**
     * Called to start a component.
     */
    virtual bool begin() = 0;

   protected:
    virtual ~BaseComponent() = default;

    BaseComponent(bool singleton = true) {
    }

    /**
     * Adds a component to the common store.
     */
    template <typename T>
    void add_component(T *c) {
        if (!component_store)
            return;

        BaseComponent *bc = static_cast<BaseComponent *>(c);
        bc->id = hash_type<T>();
        component_store->push_back(bc);
    }

    /**
     * Checks whether a component with the passed class type exists in the store.
     */
    template <typename T>
    bool has_component() {
        return get_component<T>() != 0;
    }

    /**
     * Retrieves a point
     */
    template <typename T>
    T *get_component() {
        size_t h = hash_type<T>();
        for (auto c : *component_store) {
            if (c->id == h)
                return static_cast<T *>(c);
        }

        return nullptr;
    }

};