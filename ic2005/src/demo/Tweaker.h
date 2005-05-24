#ifndef __TWEAKER_H
#define __TWEAKER_H

namespace dingus {
	class CUIDialog;
};


namespace tweaker {

	struct SOptions {
		SOptions();
		void	apply();

		bool	wireframe;
		bool	shadows;
		bool	reflections;
		bool	dof;
		bool	normalmaps;
		bool	ao;
		bool	funky;
	};


	void	init();
	void	shutdown();

	void	show();
	void	hide();
	CUIDialog& getDlg();
	bool	isVisible();
	
	const SOptions& getOptions();
};


#endif
