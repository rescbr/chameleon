Module:	AMDGraphicsEnabler

Description: the GraphicsEnabler ATI code ( > r784) ported to a module.
			 This code has no support for "legacy" ATI cards.
			 If your HD card was supported on the old code and you find
			 it missing on this, please file an issue at:
			 http://forge.voodooprojects.org/p/chameleon/issues/
			 The same applies to any missing card on this code.
			 Only cards known to work will be added.

Dependencies: none

Keys: GraphicsEnabler	Yes/No (enabled by default)
						Disable GraphicsEnabler patch.
						
	  ATYbinimage		Yes/No (enabled by default)
						Disable adding VBIOS read from "legacy space or PCI ROM"
						to "ATY,bin_image" ioreg property. ???confirm
						
	  UseAtiROM			Yes/No (disabled by default)
						Enable the use of a custom VBIOS ROM image added
						to "ATY,bin_image" ioreg property. ??? Azi: confirm
						
	  AtiConfig			Used to test a framebuffer different from the default one.

		
Adaptation of Meklort's work.