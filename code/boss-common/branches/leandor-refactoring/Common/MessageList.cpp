#include "Mods.h"

namespace boss {

void MessageList::Add( const string& message, bool replacingAll )
{
	if (message.empty())
		return;

	if (replacingAll) {
		items.clear();
	}

	items.push_back(message);
}

}