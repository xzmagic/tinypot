#include "BigPotSubtitle.h"

BigPotSubtitle::BigPotSubtitle()
{
	fontname_ = config_->getString("sub_font");
	if (!fileExist(fontname_))
	{
#ifdef _WIN32
		fontname_ = "c:/windows/fonts/msyh.ttc";
#endif
#ifdef __APPLE__
		fontname_ = "/System/Library/Fonts/STHeiti Medium.ttc";
#endif
	}
}


BigPotSubtitle::~BigPotSubtitle()
{
}


