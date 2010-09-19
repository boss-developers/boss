#pragma once

#include <string>
#include <iostream>

#include <boost/filesystem.hpp>

#include "Support/Container.h"
#include "Support/Helpers.h"

namespace boss {

	using namespace std;
	namespace fs = boost::filesystem;

	class ModList;
	class Mod;
	class MessageList;

	//Thrown when the changing of sorting positions fails for any reason.
	class SortFailed : public std::exception 
	{
	public:
		SortFailed(const string& what) throw (): std::exception(what.c_str()) {};
	};

	//Defines a particular Mod ordering
	class Ordering : public ListWrapper<Mod*> 
	{
	public:
		// Generates a clone of the given other Ordering list containing 
		//	the same Mods but in possibly different positions.
		Ordering(const Ordering& other) : ListWrapper(other) {}

		// Sorts the list by last-write time order
		void Sort();

		// Prints the current ordering to the given output stream
		ostream& Save(ostream& out);

	private:
		// ModList needs to access the Add method here
		friend ModList;

		// Default constructor is required since we redefined the copy constructor above
		Ordering() : ListWrapper() 	{}
		
		// Adds a mod to the list
		void Add(Mod* mod);
	};

	//List of messages attached to Mods
	class MessageList : public VectorWrapper<string> {
	public:
		void Add(const string& message, bool replacingAll = false);
	};

	//Class to store and manipulate the 'modlist'.
	class ModList : public MapWrapper<string, Mod*> {
	public:
		//Date comparison, used for sorting mods in modlist class.
		static bool SortByDate(Mod* mod1, Mod* mod2);

	public:
		ModList()
		{ 
			Load(); 
		}

	public:
		Ordering& LoadOrder()
		{
			return loadorder;
		}

	public:
		//Debug output function.
		void Print(ostream& out);

		//Save mod list to modlist.txt. Backs up old modlist.txt as modlist.old first.
		int Save();
		
		//Look for a mod in the modlist, even if the mod in question is ghosted.
		int IndexOf(string mod);

		//Changes the load order position for the specified mod.
		void SetPosition(const string mod, SizeType pos);
		void SetPosition(Iterator item, SizeType pos);

		//Adds a new message to the list of mod's messages, optionally replacing all already there.
		void AddMessage(const string mod, const string message, bool replacingAll = false);
		void AddMessage(Iterator item, const string message, bool replacingAll = false);

	public:		
		//Look for a mod in the modlist, even if the mod in question is ghosted, but returns an iterator to it instead of its index.
		Iterator Find(const string mod);

	private:
		//Adds mods in directory to modlist in date order (AKA load order).
		void Load();
		void Add(const fs::path& path);

	private:
		Ordering loadorder;
	};

	// Holds the Mod information
	class Mod {
	public:
		Mod(const fs::path& path);

	public:
		// Prints the Mod's name to the output stream
		ostream& operator<<(ostream& os) const
		{
			return os << this->name;
		}

	public:
		const string& Filename() const
		{
			return path.filename();
		}

		const fs::path& Path() const
		{
			return this->path;
		}

		const string& Name() const
		{
			return name;
		}

		const string& ID() const
		{
			return id;
		}

		const MessageList& Messages() const
		{
			return this->messages;
		}

		const bool IsGhosted() const 
		{
			return this->ghosted;
		}

		const time_t& LastWriteTime() const 
		{
			return this->lastWriteTime;
		}

		void LastWriteTime(const time_t& value) throw (SortFailed);

	public:
		// Gets the corresponding Mod's ID from the given path
		static const string& ID(const fs::path& path);

		// Gets the corresponding Mod's name from the given path
		static const string& Name(const fs::path& path);

		// Returns true if the specified Mod has the given id
		static bool Equals(const Mod* mod, const string id)
		{
			return mod && mod->id == Tidy(id);
		}

		// Returns true if both Mod are the same
		static bool Equals(const Mod* mod1, const Mod* mod2)
		{
			return (mod1 == mod2) || (mod1->id == mod2->id);
		}

		static bool SortByLastWrite(const Mod* mod1, const Mod* mod2)
		{
			return (mod1 != mod2) && (::difftime(mod1->lastWriteTime, mod2->lastWriteTime) < 0);
		}

		static bool IsMod(const fs::path& path);

	public:
		void AddMessage(const string& message, bool replacingAll = false)
		{
			messages.Add(message, replacingAll);
		}

	private:
		MessageList messages;
		fs::path path;
		string name;
		string id;
		time_t lastWriteTime;
		bool ghosted;
	};
}