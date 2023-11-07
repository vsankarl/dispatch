#pragma once

#include <functional>
#include <vector>
#include <memory>
#include "utils.h"
#include "errors.h"


namespace EVManager
{
    // Simple interface that helps to 
    // store it as a collection
    class EventActionBase
    {
        public:
            EventActionBase() = default; 
            virtual ~EventActionBase() = default;
    };

    template< typename R, typename ... Args >
    class EventAction : public EventActionBase
    {
        public:
            EventAction() = default; 
            EventAction( R ( * action )( Args ... ) ):m_action( action )
            {
            }
            virtual ~EventAction() = default;
            R operator( ) ( Args && ... args )
            {
                return m_action(std::forward<Args>(args)...);
            }

        protected:
            std::function< R ( Args ... ) >   m_action;
    };

    // TODO: Multithreading support
    class EventActionCollection
    {
        public:
            EventActionCollection():m_cancelled(false)
            {
            }

            template <typename R, typename ... Args>
            long Add( R ( * action )( Args ...) )
            {
                EventActionBasePtrT evActionPtr = std::make_shared< EventAction< R, Args ...> > (action) ;
                if ( evActionPtr.get() == nullptr )
                {
                    Logger::Log("Failed to allocate object %d", ERR_MEMORY_FAILURE );
                    return ERR_MEMORY_FAILURE;
                }
                m_allActions.push_back(evActionPtr);
                return 0;
            }

            template <typename R, typename ... Args>
            long Invoke( Args && ... args )
            {
                m_cancelled = false;
                for ( auto &it : m_allActions )
                {
                    if ( m_cancelled )
                    {
                        Logger::Log("Skip processing rest the actions" );
                        break;
                    }
 
                    // Ignore failures and continue with the rest of the actions.
                    HandleEvent<R>(it, std::forward<Args>(args)... );
                }

                return 0;
            }

            long Cancel()
            {
                m_cancelled = true;
                return 0;
            }

        private:

            typedef std::shared_ptr< EventActionBase >     EventActionBasePtrT;
            typedef std::vector< EventActionBasePtrT >     EventActionVT;
            EventActionVT           m_allActions;
            bool                   m_cancelled;

            template<typename R, typename ... Args>
            long HandleEvent( EventActionBasePtrT & ievActionPtr, Args && ... args )
            {
                // Note: Enforce a strict matching as opposed to any conversion that can happen
                //       for a type. Using static_pointer_cast can get away with that limitation
                //       but it can lead to other errors for completly mimatched type.

                std::shared_ptr< EventAction< R, Args ... > > evActionPtr = std::dynamic_pointer_cast
                                                                                < EventAction< R, Args ... > >
                                                                                ( ievActionPtr );

               if ( evActionPtr.get() == nullptr ) 
                {
                    Logger::Log("Invalid parameter type. %d", ERR_INVALID_PARAMETER_TYPE );
                    return ERR_INVALID_PARAMETER_TYPE;
                }
                ( * evActionPtr )( std::forward<Args>(args)... );
 
                return 0;
            }
    };
}