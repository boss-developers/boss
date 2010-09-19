#pragma once

#include "Mods.h"

#include <list>
#include <iostream>

namespace boss {

void Ordering::Add( Mod* mod )
{
	items.push_back(mod);
}

void Ordering::Sort()
{
	items.sort(Mod::SortByLastWrite);
}

ostream& Ordering::Save( ostream& out )
{
	for (Iterator iter = begin(); iter != end(); iter++) {
		out << *iter << endl;
	}
}

}