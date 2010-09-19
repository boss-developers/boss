#include "Mods.h"

#include <algorithm>

namespace boss {

	//Date comparison, used for sorting mods in modlist class.
	bool ModList::SortByDate(Mod* mod1, Mod* mod2) 
	{
		return difftime(mod1->LastWriteTime(), mod2->LastWriteTime()) < 0;
	}

	//Adds mods in directory to modlist in date order (AKA load order).
	void ModList::Load() 
	{
		fs::path p(".");

		if (fs::is_directory(p)) {
			for (fs::directory_iterator itr(p); itr != fs::directory_iterator(); ++itr) {
				if (fs::is_regular_file(itr->status()) && (fs::extension(itr->filename())==".esp" || fs::extension(itr->filename())==".esm" || fs::extension(itr->filename())==".ghost")) {
					Add(*itr);
				}
			}
		}

		loadorder.Sort();
	}

	void ModList::Add(const fs::path& path)
	{
		const string& id = Mod::ID(path);
		
		ConstIterator iter = items.find(id);

		if (!IsValid(iter)) {
			Mod* mod = new Mod(path);
			
			items[id] = mod;
			loadorder.Add(mod);
		}
	}

	//Save mod list to modlist.txt. Backs up old modlist.txt as modlist.old first.
	int ModList::Save() 
	{
		ofstream modlist;
		try {
			//There's a bug in the boost rename function - it should replace existing files, but it throws an exception instead, so remove the file first.
			//This bug will be fixed in BOOST 1.45, but that will break the AddMods() function and possibly other things, so be sure to change that when we upgrade.
			if (fs::exists("BOSS\\modlist.old")) fs::remove("BOSS\\modlist.old");
			if (fs::exists("BOSS\\modlist.txt")) fs::rename("BOSS\\modlist.txt", "BOSS\\modlist.old");
		} catch(fs::filesystem_error e) {
			//Couldn't rename the file, print an error message.
			return 1;
		}
		modlist.open("BOSS\\modlist.txt");
		//Provide error message if it can't be written.
		if (modlist.fail()) {
			return 2;
		}

		loadorder.Save(modlist);

		return 0;
	}

	//Debug output function.
	void ModList::Print(ostream& out) 
	{
		for (Iterator iter = begin(); iter != end(); iter++) {
			const Mod& mod = *iter->second;

			out << mod << "<br />" << endl;
		}
	}

	//Look for a mod in the modlist, even if the mod in question is ghosted.
	int ModList::IndexOf(string modname) 
	{
		Iterator iter = Find(modname);

		if (IsValid(iter))
		if (iter != items.end())
			return iter - items.begin();

		return -1;
	}

	//Look for a mod in the modlist, even if the mod in question is ghosted, but returns an iterator to it instead of its index.
	ModList::Iterator ModList::Find(const string modname) 
	{
		const string& id = Tidy(modname);
		return items.find(id);
	}

	void ModList::SetPosition(const string mod, SizeType pos)	
	{
		Iterator ptr = Find(mod);
		SetPosition(ptr, pos);
	}

	void ModList::SetPosition(Iterator iter, SizeType pos)	
	{

		if (!IsValid(iter)) 
			return;

		items.erase(iter);

		if (pos > size())
			pos = size();

		items.insert(items.begin() + pos, *iter);
	}

	void ModList::AddMessage(const string mod, const string message, bool replacingAll) 
	{
		Iterator iter = Find(mod);
		AddMessage(iter, message, replacingAll);
	}

	void ModList::AddMessage( Iterator iter, const string message, bool replacingAll )
	{
		if (IsValid(iter)){
			iter->second->AddMessage(message, replacingAll);
		}
	}
}