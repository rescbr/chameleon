Module:	GraphicsEnabler

Description: the GraphicsEnabler nVidia code ported to a module.
			 If your card is supported and you find it missing
			 on this code, please file an issue at:
			 http://forge.voodooprojects.org/p/chameleon/issues/
			 Only cards known to work will be added.

Dependencies: none

Keys: GraphicsEnabler	Yes/No (enabled by default)
						Disable GraphicsEnabler patch.
						
	  UseNvidiaROM		Yes/No (disabled by default)
						Enable the use of a custom VBIOS ROM image.
						
	  VBIOS				Yes/No (disabled by default)
						Adds "vbios" property to ioreg. ??? Azi: confirm


Adaptation of Meklort's work.

TODO: review "nv_chipsets_t NVKnownChipsets"