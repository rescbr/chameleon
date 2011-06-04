Module:	AMDGraphicsEnabler

Description: the GraphicsEnabler ATI code ( > r784) ported to a module.
			 This code has no support for "legacy" ATI cards.
			 If your HD card was supported on the old code and you find
			 it missing on this, please file an issue at:
			 http://forge.voodooprojects.org/p/chameleon/issues/
			 The same applies to any missing card on this code.
			 Only known to work cards will be added. 

Dependencies: none

Keys: GraphicsEnabler	Yes/No (enabled by default)
						Disable GraphicsEnabler patch.
	  ATYbinimage		Yes/No (enabled by default)
						Disable adding VBIOS read from "legacy space or PCI ROM"
						to "ATY,bin_image" ioreg property.
	  UseAtiROM			Yes/No (disabled by default)
						Enable the use of a costume VBIOS ROM image added
						to "ATY,bin_image" ioreg property.
	  AtiConfig			Used to test a framebuffer different from the default one.

		
Adaptation of Meklort's work.

TODO:
- fix malloc error (ati.c #1235) when compiling with XCode 4
- merge ATiGraphicsEnabler into this module ??
- finish styling ati_reg.h
- review "radeon_card_info_t radeon_cards"