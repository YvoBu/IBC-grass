
#include <cassert>
#include <iostream>
#include "itv_mode.h"
#include "Parameters.h"
#include "Traits.h"
#include "Environment.h"

// Input Files
Parameters::Parameters() :
		weekly(0), ind_out(0), PFT_out(2), srv_out(1), trait_out(1), aggregated_out(1),
		AboveCompMode(asympart), BelowCompMode(sym), stabilization(version1), mode(communityAssembly),
		Tmax_monoculture(10),
		ITV(off), ITVsd(0),
		Tmax(100),
		seedMortality(0.5), winterDieback(0.5), backgroundMortality(0.007), litterDecomp(0.5),
		meanARes(100), meanBRes(100),
		rametEstab(1),
		AbvGrazProb(0), AbvPropRemoved(0), BiteSize(0.5), MassUngrazable(15300),
		BelGrazProb(0), BelGrazPerc(0), BelGrazResidualPerc(0),
		BelGrazAlpha(2), BelGrazHistorySize(60),
		CutHeight(0), NCut(0),
		CatastrophicDistYear(100), CatastrophicDistWeek(20),
		CatastrophicPlantMortality(0),
		Aampl(0), Bampl(0),
		SeedInput(0), SeedRainType(0)
{

}

//-----------------------------------------------------------------------------


