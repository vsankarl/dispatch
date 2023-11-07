#pragma once

#include <string>
#include <unordered_map>
#include "eventaction.h"

namespace EVManager
{
    // Store any event type - string, int, etc.
    // TODO: For custom types the template need to take 
    // in comparator function as anpther template parameter.
    // Thread safety need to be incorporated.
    template <typename EventType>
    class EventBus
    {
        public:

        // A single event can map to multiple functions with difference
        // signatures. 
        template <typename R, typename ... Args>
        long Add( EventType t, R ( * action )( Args ...) )
        {
            if ( !action ) 
            {
                Logger::Log("Invalid action paramter %d", ERR_INVALID_PARAMETER );
                return ERR_INVALID_PARAMETER;
            }

            EventActionCollectionPtrT evActionCollectionPtr(nullptr); 
            long result = GetEventCollection(t, evActionCollectionPtr);
            if ( result != 0 )
            {
                // Event not found 
                evActionCollectionPtr = std::make_shared< EventActionCollection > () ;
                if ( evActionCollectionPtr.get() == nullptr )
                {
                    Logger::Log("Failed to allocate object %d", ERR_MEMORY_FAILURE );
                    return ERR_MEMORY_FAILURE;
                }
                m_allEvents[ t ] = evActionCollectionPtr ;
            }   

            result = evActionCollectionPtr->Add( action );

            return result;
        }

        // Enforces strict type matching, no implicit conversion allowed
        // ex - const char * ==> string, float -> int. Caller need to make
        // sure the type of parameter matches the callback function provided. 
        // Caller designs the function prototype so we leave to the caller 
        // to pass in the right type. Any mismatch would return error. 
        // 
        // Add has the ability to map one event to many function prototypes.
        // However "Invoke" in a single call take only one function signature.
        // This is to align with the requirements while still having some 
        // level of maximum extensibility. Over time additional interface can 
        // be evolved such that it can have array of varying function parameters 
        // for an event.
        // For now any mismatched function signature are skipped.

        // C++ perfect forwarding notion is leveraged to handle paramerer references. 
        // lvalues get deduced as lvalue reference. Ex int i = 10
        // and passing "i" to Invoke gets "i" deduced as "int &". If the function prototype takes 
        // the form "void foo(int i)" then our dynamic_cast fails ( int & != int). So the caller
        // need to typecast "i" ==> (int)i so that compiler deduces the type as "int" (as 
        // opposed to int &) which matches the function signature and dynamic_cast happens
        // properly. 
        // Essentially for passing lvalues as rvalue caller need to type cast.
        // See Invoke on "printStdString", Add in various forms.
        template <typename R, typename ... Args>
        long Invoke( EventType t, Args && ... args)
        {
            EventActionCollectionPtrT evActionCollectionPtr(nullptr);
            long result = GetEventCollection(t, evActionCollectionPtr);
            if (result != 0)
            {
                Logger::Log("Failed to get the collection object %d", result);
                return result;
            }
            result = evActionCollectionPtr->Invoke< R >( std::forward<Args>(args)... );    
            
            return result;
        }

        long Cancel( EventType t )
        {
            Logger::Log("Cancelling all the events %s", t );
            EventActionCollectionPtrT evActionCollectionPtr(nullptr);
            long result = GetEventCollection(t, evActionCollectionPtr);
            if (result != 0)
            {
                Logger::Log("Failed to get the collection object %d", result);
                return result;
            }

            result = evActionCollectionPtr->Cancel();    
            return result;
        }

        private:

            typedef std::shared_ptr< EventActionCollection >     EventActionCollectionPtrT;
            typedef typename std::unordered_map< EventType, EventActionCollectionPtrT > EventMapT;
            typedef typename std::unordered_map< EventType, EventActionCollectionPtrT >::iterator EventMapIterT;

            EventMapT  m_allEvents;

            inline long GetEventCollection( EventType t, EventActionCollectionPtrT & evActionCollectionPtr )
            {
                EventMapIterT evIt = m_allEvents.find(t);
                if ( evIt == m_allEvents.end() )
                    return ERR_EVENT_NOT_FOUND;

                if ( (evIt->second).get() == nullptr )
                    return ERR_NO_EVENT_COLLECTION_FOUND;
 
                evActionCollectionPtr = evIt->second;
 
                return 0;
            }
     };
}