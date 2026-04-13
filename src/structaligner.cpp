#include <pro.h>
#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <kernwin.hpp>
#include <typeinf.hpp>

#include "scriptmgr.h"
#include "plugin.h"

#include <concepts>
#include <format>
#include <string>
#include <vector>

// Hey you!
// Don't mind this file, but it's here in case you want it.
// It works, but IDA freezes for a solid 20 minutes when you run it.
// IDAPython must do some thread magic to not have the entire program freeze when you run a script.
// This is more or less of a "fire and forget" script, so if you wanna build and use it, go ahead.
// Just run it and then go touch some grass for a while. It's good for you.

class StructAlign : public CallableScript
{
public:
	StructAlign(const char *pName)
		: CallableScript(pName, pName, nullptr, 0) {}
	StructAlign(const char *pName, const char *pLabel, const char *pPath = nullptr, int actionflags = 0)
		: CallableScript(pName, pLabel, pPath, actionflags) {}

	virtual int Activate(action_update_ctx_t *ctx)
	{
		tinfo_t check;
		if (!check.get_named_type(nullptr, "CBaseEntity"))
		{
			warning("You should only run this script after importing a netprop/datamap dump");
			return 0;
		}

		msg("Warning: This will take a while...\n");

		const char *name = first_named_type(nullptr, NTF_TYPE);
		while (name != nullptr)
		{
			tinfo_t struc_tif;
			if (struc_tif.get_named_type(nullptr, name) && struc_tif.is_struct())
			{
				udt_type_data_t udt;
				if (struc_tif.get_udt_details(&udt))
				{
					uint64 total_bits = (uint64)udt.total_size * 8;
					uint64 curr_bits = 0;

					// Collect gap offsets (in bits) before modifying the type
					std::vector<uint64> gap_bits;
					for (size_t i = 0; i < udt.size(); i++)
					{
						while (curr_bits < udt[i].offset)
						{
							gap_bits.push_back(curr_bits);
							curr_bits += 8;
						}
						curr_bits = udt[i].end();
					}
					while (curr_bits < total_bits)
					{
						gap_bits.push_back(curr_bits);
						curr_bits += 8;
					}

					// Fill in gap bytes
					for (uint64 bit_off : gap_bits)
					{
						std::string gapname = std::format("gap_{:x}", (size_t)(bit_off / 8));
						struc_tif.add_udm(gapname.c_str(), tinfo_t(BTF_BYTE), bit_off);
					}
				}
			}
			name = next_named_type(nullptr, name, NTF_TYPE);
		}

		msg("Done!\n");
		return 0;
	}
};

static StructAlign script("Align imported structures");