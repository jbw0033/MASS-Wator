#ifndef DERIVEDPLACE_H
#define DERIVEDPLACE_H

#include <iostream>
#include "Place.h"
#define FUNCTION 0 // FUNCTION ID

class DerivedPlace : public Place {
public:

    /**
     * 1: CONSTRUCTOR DESIGN
     * @param argument
     */
    DerivedPlace(void *argument) : Place(argument) {
        // START OF USER IMPLEMENTATION 
        // END OF USER IMPLEMENTATION   
    }

    /**
     * 2: CALLALL DESIGN
     * @param functionId
     * @param argument
     * @return 
     */
    virtual void *callAll(int functionId, void *argument) {
        switch (functionId) {
                // START OF USER IMPLEMENTATION
            case FUNCTION: return function(argument);
                // END OF USER IMPLEMNTATION
        }
        return NULL;
    };

private:

    /**
     * START OF USER IMPLEMENTATION
     * @param argument
     * @return 
     */
    void *function(void *argument) {
        return NULL;
    }
    // END OF USER IMPLEMENTATION
};

#endif

/**
 * 
 */
extern "C" Place* instantiate(void *argument) {
    return new DerivedPlace(argument);
}

/**
 * 
 */
extern "C" void destroy(Place *object) {
    delete object;
}

