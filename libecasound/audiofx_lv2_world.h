#ifndef ECA_LV2_WORLD_H
#define ECA_LV2_WORLD_H

#if ECA_USE_LIBLILV

#include <lilv/lilvmm.hpp>



class ECA_LV2_WORLD {

	static ECA_LV2_WORLD i;

public:
	static LilvWorld* World();
	static LilvNode* AudioClassNode();
	static LilvNode* ControlClassNode();
	static LilvNode* InputClassNode();
	static LilvNode* OutputClassNode();
	static LilvNode* InPlaceBrokenNode();
	static LilvNode* PortToggledNode();
	static LilvNode* PortIntegerNode();
	static LilvNode* PortLogarithmicNode();
	static LilvNode* PortSamplerateDependentNode();
	static LilvNode* PortConnectionOptionalNode();

private:
	ECA_LV2_WORLD();
	~ECA_LV2_WORLD();
	LilvWorld* lilvworld;
	LilvNode* audioclassnode;
	LilvNode* controlclassnode;
	LilvNode* inputclassnode;
	LilvNode* outputclassnode;
	LilvNode* inplacebrokennode;
	LilvNode* porttogglednode;
	LilvNode* portintegernode;
	LilvNode* portlogarithmicnode;
	LilvNode* portsampleratedependentnode;
	LilvNode* portconnectionoptionalnode;
};

#endif /* ECA_USE_LIBLILV */
#endif // ECA_LV2_WORLD_H
