// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __ASSERT_HELP_H
#define __ASSERT_HELP_H

#define ASSERT_MSG( condition, message ) assert( (condition) || !message )
#define ASSERT_FAIL_MSG( message ) assert( !message )
#define ASSERT_NOT_IMPL ASSERT_FAIL_MSG("not implemented")

#endif
