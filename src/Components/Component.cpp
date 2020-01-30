#include "components/component.h"

namespace game
{
	component::component(actor& owner, const int update_order)
		:owner{owner}, update_order_{update_order}
	{
	}
}
