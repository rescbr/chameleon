/*
 * Copyright 2008 mackerintel
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "acpi.h"
#include "efi_tables.h"
#include "fake_efi.h"
#include "dsdt_patcher.h"
#include "platform.h"

#ifndef DEBUG_DSDT
#define DEBUG_DSDT 0
#endif

#if DEBUG_DSDT==2
#define DBG(x...)  {printf(x); sleep(1);}
#elif DEBUG_DSDT==1
#define DBG(x...)  printf(x)
#else
#define DBG(x...)
#endif

struct acpi_2_fadt *
patch_fadt(struct acpi_2_fadt *fadt, void *new_dsdt, bool UpdateFADT)
{
    extern void setupSystemType(); 
	
    struct acpi_2_fadt *fadt_mod;
    struct acpi_2_fadt *fadt_file = (struct acpi_2_fadt *)acpiLoadTable(kFADT);
    bool fadt_rev2_needed = false;
    bool fix_restart;
    const char * value;

    // Restart Fix
    if (Platform.CPU.Vendor == 0x756E6547) {	/* Intel */
        fix_restart = true;
        getBoolForKey(kRestartFix, &fix_restart, &bootInfo->bootConfig);
    } else {
        verbose ("Not an Intel platform: Restart Fix not applied !!!\n");
        fix_restart = false;
    }

    if (fix_restart)
        fadt_rev2_needed = true;

    // Allocate new fadt table
    if ((UpdateFADT) && (((fadt_file) && (fadt_file->Length < sizeof(struct acpi_2_fadt))) ||
                         ((!fadt_file) && (fadt->Length < sizeof(struct acpi_2_fadt)))))
    {
        fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(sizeof(struct acpi_2_fadt));

        if (fadt_file)
            memcpy(fadt_mod, fadt_file, fadt_file->Length);
        else
            memcpy(fadt_mod, fadt, fadt->Length);

        fadt_mod->Length = sizeof(struct acpi_2_fadt);
        fadt_mod->Revision = 0x04; // FADT rev 4
        fadt_mod->RESET_REG = acpiFillGASStruct(0, 0);
        fadt_mod->RESET_VALUE = 0;
        fadt_mod->Reserved2[0] = 0;
        fadt_mod->Reserved2[1] = 0;
        fadt_mod->Reserved2[2] = 0;
        fadt_mod->X_PM1a_EVT_BLK = acpiFillGASStruct(fadt_mod->PM1a_EVT_BLK, fadt_mod->PM1_EVT_LEN);
        fadt_mod->X_PM1b_EVT_BLK = acpiFillGASStruct(fadt_mod->PM1b_EVT_BLK, fadt_mod->PM1_EVT_LEN);
        fadt_mod->X_PM1a_CNT_BLK = acpiFillGASStruct(fadt_mod->PM1a_CNT_BLK, fadt_mod->PM1_CNT_LEN);
        fadt_mod->X_PM1b_CNT_BLK = acpiFillGASStruct(fadt_mod->PM1b_CNT_BLK, fadt_mod->PM1_CNT_LEN);
        fadt_mod->X_PM2_CNT_BLK = acpiFillGASStruct(fadt_mod->PM2_CNT_BLK, fadt_mod->PM2_CNT_LEN);
        fadt_mod->X_PM_TMR_BLK = acpiFillGASStruct(fadt_mod->PM_TMR_BLK, fadt_mod->PM_TMR_LEN);
        fadt_mod->X_GPE0_BLK = acpiFillGASStruct(fadt_mod->GPE0_BLK, fadt_mod->GPE0_BLK_LEN);
        fadt_mod->X_GPE1_BLK = acpiFillGASStruct(fadt_mod->GPE1_BLK, fadt_mod->GPE1_BLK_LEN);
        verbose("Converted ACPI V%d FADT to ACPI V4 FADT\n", (fadt_file) ? fadt_file->Revision : fadt->Revision);
    } else {
        if (((!fadt_file) && ((fadt->Length < 0x84) && (fadt_rev2_needed))) ||
            ((fadt_file) && ((fadt_file->Length < 0x84) && (fadt_rev2_needed))))
        {
            fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(0x84);

            if (fadt_file)
                memcpy(fadt_mod, fadt_file, fadt_file->Length);
            else
                memcpy(fadt_mod, fadt, fadt->Length);

            fadt_mod->Length   = 0x84;
            fadt_mod->Revision = 0x02; // FADT rev 2 (ACPI 1.0B MS extensions)
        }
        else
        {
            if (fadt_file)
            {
                fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(fadt_file->Length);
                memcpy(fadt_mod, fadt_file, fadt_file->Length);
            } else {
                fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(fadt->Length);
                memcpy(fadt_mod, fadt, fadt->Length);
            }
        }
    }
    // Determine system type / PM_Model
    if ( (value=getStringForKey(kSystemType, &bootInfo->bootConfig))!=NULL)
    {
        if (Platform.Type > 6)  
        {
            if(fadt_mod->Preferred_PM_Profile<=6)
                Platform.Type = fadt_mod->Preferred_PM_Profile; // get the fadt if correct
            else 
                Platform.Type = 1;		/* Set a fixed value (Desktop) */
            verbose("Error: system-type must be 0..6. Defaulting to %d !\n", Platform.Type);
        }
        else
            Platform.Type = (unsigned char) strtoul(value, NULL, 10);
    }
    // Set Preferred_PM_Profile from System-type if only if user wanted this value to be forced
    if (fadt_mod->Preferred_PM_Profile != Platform.Type) 
    {
        if (value) 
        { // user has overriden the SystemType so take care of it in FACP
            verbose("FADT: changing Preferred_PM_Profile from 0x%02x to 0x%02x\n", fadt_mod->Preferred_PM_Profile, Platform.Type);
            fadt_mod->Preferred_PM_Profile = Platform.Type;
        }
        else
        { // Preferred_PM_Profile has a different value and no override has been set, so reflect the user value to ioregs
            Platform.Type = fadt_mod->Preferred_PM_Profile <= 6 ? fadt_mod->Preferred_PM_Profile : 1;
        }  
    }
    // We now have to write the systemm-type in ioregs: we cannot do it before in setupDeviceTree()
    // because we need to take care of facp original content, if it is correct.
    setupSystemType();

    // Patch FADT to fix restart
    if (fix_restart)
    {
        fadt_mod->Flags|= 0x400;
        fadt_mod->RESET_REG = acpiFillGASStruct(0x0cf9, 1);
        fadt_mod->RESET_VALUE = 0x06;
        verbose("FADT: Restart Fix applied !\n");
    }

    // Patch FACS Address
    fadt_mod->FIRMWARE_CTRL=(uint32_t)fadt->FIRMWARE_CTRL;
    if ((uint32_t)(&(fadt_mod->X_FIRMWARE_CTRL))-(uint32_t)fadt_mod+8<=fadt_mod->Length)
        fadt_mod->X_FIRMWARE_CTRL=(uint32_t)fadt->FIRMWARE_CTRL

            // Patch DSDT Address
            DBG("DSDT: Old @%x,%x, ",fadt_mod->DSDT,fadt_mod->X_DSDT);

    fadt_mod->DSDT=(uint32_t)new_dsdt;
    if ((uint32_t)(&(fadt_mod->X_DSDT))-(uint32_t)fadt_mod+8<=fadt_mod->Length)
        fadt_mod->X_DSDT=(uint32_t)new_dsdt;

    DBG("New @%x,%x\n",fadt_mod->DSDT,fadt_mod->X_DSDT);

    // Correct the checksum
    fadt_mod->Checksum=0;
    fadt_mod->Checksum=256-checksum8(fadt_mod,fadt_mod->Length);

    return fadt_mod;
}

/* Setup ACPI without replacing DSDT. */
int setupAcpiNoMod()
{
    //	addConfigurationTable(&gEfiAcpiTableGuid, acpiGetAddressOfTable10(), "ACPI");
    //	addConfigurationTable(&gEfiAcpi20TableGuid, acpiGetAddressOfTable20(), "ACPI_20");
    /* XXX aserebln why uint32 cast if pointer is uint64 ? */
    acpi10_p = (uint32_t) acpiGetAddressOfTable10();
    acpi20_p = (uint32_t) acpiGetAddressOfTable20();
    addConfigurationTable(&gEfiAcpiTableGuid, &acpi10_p, "ACPI");
    if(acpi20_p) addConfigurationTable(&gEfiAcpi20TableGuid, &acpi20_p, "ACPI_20");
    return 1;
}


/* Setup ACPI. Replace DSDT if DSDT.aml is found */
int setupAcpi(void)
{
    int version;
    void *new_dsdt=NULL, *new_content=NULL;

    bool update_acpi=false, gen_xsdt=false;
    bool hpet_replaced=false, sbst_replaced=false, ecdt_replaced=false, asft_replaced=false, dmar_replaced=false, apic_replaced=false, mcfg_replaced=false;
    bool hpet_added=false, sbst_added=false, ecdt_added=false, asft_added=false, dmar_added=false, apic_added=false, mcfg_added=false;

    int curssdt=0, loadtotssdt=0, totssdt=0, newtotssdt=0;

    {
        bool tmpval;

        update_acpi=getBoolForKey(kUpdateACPI, &tmpval, &bootInfo->bootConfig)&&tmpval;
    }

    // Load replacement ACPI tables if they provided by user according to the drop table list
    acpiLoadUserTables();

    if (!(new_dsdt=acpiLoadTable("DSDT")))
        return setupAcpiNoMod();

    DBG("New ACPI tables Loaded in memory\n");
			
    // Do the same procedure for both versions of ACPI
    for (version=0; version<2; version++) {
        struct acpi_2_rsdp *rsdp, *rsdp_mod, *rsdp_conv=(struct acpi_2_rsdp *)0;
        struct acpi_2_rsdt *rsdt, *rsdt_mod;
        struct acpi_2_xsdt *xsdt_conv = (struct acpi_2_xsdt *)0;
        int rsdplength;

        // Find original rsdp
        rsdp=(struct acpi_2_rsdp *) (version? 
                                     acpiGetAddressOfTable20() : 
                                     acpiGetAddressOfTable10());
        if ((update_acpi) && (rsdp->Revision == 0))
        {
            rsdp_conv = (struct acpi_2_rsdp *)AllocateKernelMemory(sizeof(struct acpi_2_rsdp));
            memcpy(rsdp_conv, rsdp, 20);

            /* Add/change fields */
            rsdp_conv->Revision = 2; /* ACPI version 3 */
            rsdp_conv->Length = sizeof(struct acpi_2_rsdp);

            /* Correct checksums */
            rsdp_conv->Checksum = 0;
            rsdp_conv->Checksum = 256-checksum8(rsdp_conv, 20);
            rsdp_conv->ExtendedChecksum = 0;
            rsdp_conv->ExtendedChecksum = 256-checksum8(rsdp_conv, rsdp_conv->Length);

            rsdp = rsdp_conv;

            gen_xsdt = true;
            version = 1;

            addConfigurationTable(&gEfiAcpiTableGuid, NULL, "ACPI");

            verbose("Converted ACPI RSD PTR version 1 to version 3\n");
        }
        if (!rsdp)
        {
            DBG("No ACPI version %d found. Ignoring\n", version+1);
            if (version)
                addConfigurationTable(&gEfiAcpi20TableGuid, NULL, "ACPI_20");
            else
                addConfigurationTable(&gEfiAcpiTableGuid, NULL, "ACPI");
            continue;
        }
        rsdplength=version?rsdp->Length:20;

        DBG("RSDP version %d found @%x. Length=%d\n",version+1,rsdp,rsdplength);

        /* FIXME: no check that memory allocation succeeded 
         * Copy and patch RSDP,RSDT, XSDT and FADT
         * For more info see ACPI Specification pages 110 and following
         */

        if (gen_xsdt)
        {
            rsdp_mod=rsdp_conv;
        } else {
            rsdp_mod=(struct acpi_2_rsdp *) AllocateKernelMemory(rsdplength);
            memcpy(rsdp_mod, rsdp, rsdplength);
        }

        rsdt=(struct acpi_2_rsdt *)(rsdp->RsdtAddress);

        DBG("RSDT @%x, Length %d\n",rsdt, rsdt->Length);
		
        if (rsdt && (uint32_t)rsdt !=0xffffffff && rsdt->Length<0x10000)
        {
            uint32_t *rsdt_entries;
            int rsdt_entries_num;
            int dropoffset=0, i;
			
            rsdt_mod=(struct acpi_2_rsdt *)AllocateKernelMemory(rsdt->Length); 
            memcpy (rsdt_mod, rsdt, rsdt->Length);
            rsdp_mod->RsdtAddress=(uint32_t)rsdt_mod;
            rsdt_entries_num=(rsdt_mod->Length-sizeof(struct acpi_2_rsdt))/4;
            rsdt_entries=(uint32_t *)(rsdt_mod+1);

            if (gen_xsdt)
            {
                uint64_t *xsdt_conv_entries;

                xsdt_conv=(struct acpi_2_xsdt *)AllocateKernelMemory(sizeof(struct acpi_2_xsdt)+(rsdt_entries_num * 8));
                memcpy(xsdt_conv, rsdt, sizeof(struct acpi_2_rsdt));

                xsdt_conv->Signature[0] = 'X';
                xsdt_conv->Signature[1] = 'S';
                xsdt_conv->Signature[2] = 'D';
                xsdt_conv->Signature[3] = 'T';
                xsdt_conv->Length = sizeof(struct acpi_2_xsdt)+(rsdt_entries_num * 8);

                xsdt_conv_entries=(uint64_t *)(xsdt_conv+1);

                for (i=0;i<rsdt_entries_num;i++)
                {
                    xsdt_conv_entries[i] = (uint64_t)rsdt_entries[i];
                }

                xsdt_conv->Checksum = 0;
                xsdt_conv->Checksum = 256-checksum8(xsdt_conv, xsdt_conv->Length);

                rsdp->XsdtAddress = (uint32_t)xsdt_conv;

                rsdp->ExtendedChecksum = 0;
                rsdp->ExtendedChecksum = 256-checksum8(rsdp, rsdp->Length);

                verbose("Converted RSDT table to XSDT table\n");
            }

            for (i=0;i<rsdt_entries_num;i++)
            {
                char *table=(char *)(rsdt_entries[i]);
                if (!table)
                    continue;

                DBG("TABLE %c%c%c%c,",table[0],table[1],table[2],table[3]);

                /*****************/
                /* CHECK ME here:*/
                /****************/

                /* either we add a user table*/
                if ((new_content=acpiTableUserContent(table)))
                {
                    DBG("..Installing user acpi table %c%c%c%c\n", table[0],table[1],table[2],table[3]);
                    if (new_content)
                    {
                        rsdt_entries[i-dropoffset]=(uint32_t)new_content;
                        rsdt_mod->Length+=4;
                    }
                    continue;
                }
                else if (acpiIsTableDropped(table))
                { // or it is in the drop list and we don't add it at all
                    dropoffset++;
                    continue;
                }
                else // or we keep the default table
                    rsdt_entries[i-dropoffset]=rsdt_entries[i];

                /* CHECK ME HERE: for special cases */
/*
                if ((!(oem_ssdt)) && table[0]=='S' && table[1]=='S' && table[2]=='D' && table[3]=='T')
                {
                    DBG("SSDT %d found", curssdt);
                    if (new_ssdts[curssdt])
                    {
                        DBG(" and replaced");
                        rsdt_entries[i-dropoffset]=(uint32_t)new_ssdts[curssdt];
                        totssdt++;
                    }
                    DBG("\n");
                    curssdt++;
                    continue;
                }
                if (table[0]=='F' && table[1]=='A' && table[2]=='C' && table[3]=='P')
                {
                    struct acpi_2_fadt *fadt, *fadt_mod;
                    fadt=(struct acpi_2_fadt *)rsdt_entries[i];

                    DBG("FADT found @%x, Length %d\n",fadt, fadt->Length);

                    if (!fadt || (uint32_t)fadt == 0xffffffff || fadt->Length>0x10000)
                    {
                        printf("FADT incorrect. Not modified\n");
                        continue;
                    }
					
                    fadt_mod = patch_fadt(fadt, new_dsdt, update_acpi);
                    rsdt_entries[i-dropoffset]=(uint32_t)fadt_mod;
                    continue;
                }
*/
            }
            DBG("\n");

            /* CHECK ME here */
/*            if (!oem_ssdt)
            { 			
                while ((totssdt < loadtotssdt) && (curssdt < 30))
                {
                    if (new_ssdts[curssdt])
                    {
                        DBG("adding SSDT %d\n", curssdt);
                        rsdt_entries[i-dropoffset]=(uint32_t)new_ssdts[curssdt];
                        totssdt++;
                        newtotssdt++;
                        i++;
                    }
                    curssdt++;
                }
            }
*/

            // Correct the checksum of RSDT
            rsdt_mod->Length-=4*dropoffset;
            rsdt_mod->Length+=4*newtotssdt;

            DBG("RSDT: Original checksum %d, ", rsdt_mod->Checksum);

            rsdt_mod->Checksum=0;
            rsdt_mod->Checksum=256-checksum8(rsdt_mod,rsdt_mod->Length);

            DBG("New checksum %d at %x\n", rsdt_mod->Checksum,rsdt_mod);
        }
        else
        {
            rsdp_mod->RsdtAddress=0;
            printf("RSDT not found or RSDT incorrect\n");
        }

        if (version)
        {
            struct acpi_2_xsdt *xsdt, *xsdt_mod;

            // FIXME: handle 64-bit address correctly

            if (gen_xsdt)
                xsdt=xsdt_conv;
            else
                xsdt=(struct acpi_2_xsdt*) ((uint32_t)rsdp->XsdtAddress);

            DBG("XSDT @%x;%x, Length=%d\n", (uint32_t)(rsdp->XsdtAddress>>32),(uint32_t)rsdp->XsdtAddress,
                xsdt->Length);
            if (xsdt && (uint64_t)rsdp->XsdtAddress<0xffffffff && xsdt->Length<0x10000)
            {
                uint64_t *xsdt_entries;
                int xsdt_entries_num, i;
                int dropoffset=0;
                curssdt=0, totssdt=0, newtotssdt=0;
                hpet_replaced=false, hpet_added=false;
                sbst_replaced=false, sbst_added=false;
                ecdt_replaced=false, ecdt_added=false;
                asft_replaced=false, asft_added=false;
                dmar_replaced=false, dmar_added=false;
                apic_replaced=false, apic_added=false;
                mcfg_replaced=false, mcfg_added=false;

                if (gen_xsdt)
                    xsdt_mod=xsdt;
                else
                {
                    xsdt_mod=(struct acpi_2_xsdt*)AllocateKernelMemory(xsdt->Length); 
                    memcpy(xsdt_mod, xsdt, xsdt->Length);
                }

                rsdp_mod->XsdtAddress=(uint32_t)xsdt_mod;
                xsdt_entries_num=(xsdt_mod->Length-sizeof(struct acpi_2_xsdt))/8;
                xsdt_entries=(uint64_t *)(xsdt_mod+1);
                for (i=0;i<xsdt_entries_num;i++)
                {
                    char *table=(char *)((uint32_t)(xsdt_entries[i]));
                    if (!table)
                        continue;
                    xsdt_entries[i-dropoffset]=xsdt_entries[i];
                    if (drop_ssdt && table[0]=='S' && table[1]=='S' && table[2]=='D' && table[3]=='T')
                    {
                        dropoffset++;
                        continue;
                    }	
                    if (drop_hpet && table[0]=='H' && table[1]=='P' && table[2]=='E' && table[3]=='T')
                    {
                        dropoffset++;
                        continue;
                    }
                    if (drop_slic && table[0]=='S' && table[1]=='L' && table[2]=='I' && table[3]=='C')
                    {
                        dropoffset++;
                        continue;
                    }
                    if (drop_sbst && table[0]=='S' && table[1]=='B' && table[2]=='S' && table[3]=='T')
                    {
                        dropoffset++;
                        continue;
                    }
                    if (drop_ecdt && table[0]=='E' && table[1]=='C' && table[2]=='D' && table[3]=='T')
                    {
                        dropoffset++;
                        continue;
                    }					
                    if (drop_asft && table[0]=='A' && table[1]=='S' && table[2]=='F' && table[3]=='!')
                    {
                        dropoffset++;
                        continue;
                    }
                    if (drop_dmar && table[0]=='D' && table[1]=='M' && table[2]=='A' && table[3]=='R')
                    {
                        dropoffset++;
                        continue;
                    }					
                    if ((!(oem_hpet)) && table[0]=='H' && table[1]=='P' && table[2]=='E' && table[3]=='T')
                    {
                        DBG("HPET found\n");
                        if (new_hpet)
                        {
                            xsdt_entries[i-dropoffset]=(uint32_t)new_hpet;
                            hpet_replaced=true;
                        }
                        continue;
                    }					
                    if ((!(oem_sbst)) && table[0]=='S' && table[1]=='B' && table[2]=='S' && table[3]=='T')
                    {
                        DBG("SBST found\n");
                        if (new_sbst)
                        {
                            xsdt_entries[i-dropoffset]=(uint32_t)new_sbst;
                            sbst_replaced=true;
                        }
                        continue;
                    }					
                    if ((!(oem_ecdt)) && table[0]=='E' && table[1]=='C' && table[2]=='D' && table[3]=='T')
                    {
                        DBG("ECDT found\n");
                        if (new_ecdt)
                        {
                            xsdt_entries[i-dropoffset]=(uint32_t)new_ecdt;
                            ecdt_replaced=true;
                        }
                        continue;
                    }					
                    if ((!(oem_asft)) && table[0]=='A' && table[1]=='S' && table[2]=='F' && table[3]=='!')
                    {
                        DBG("ASF! found\n");
                        if (new_asft)
                        {
                            xsdt_entries[i-dropoffset]=(uint32_t)new_asft;
                            asft_replaced=true;
                        }
                        continue;
                    }					
                    if ((!(oem_dmar)) && table[0]=='D' && table[1]=='M' && table[2]=='A' && table[3]=='R')
                    {
                        DBG("DMAR found\n");
                        if (new_dmar)
                        {
                            xsdt_entries[i-dropoffset]=(uint32_t)new_dmar;
                            dmar_replaced=true;
                        }
                        continue;
                    }					
                    if ((!(oem_apic)) && table[0]=='A' && table[1]=='P' && table[2]=='I' && table[3]=='C')
                    {
                        DBG("APIC found\n");
                        if (new_apic)
                        {
                            xsdt_entries[i-dropoffset]=(uint32_t)new_apic;
                            apic_replaced=true;
                        }
                        continue;
                    }					
                    if ((!(oem_mcfg)) && table[0]=='M' && table[1]=='C' && table[2]=='F' && table[3]=='G')
                    {
                        DBG("MCFG found\n");
                        if (new_mcfg)
                        {
                            xsdt_entries[i-dropoffset]=(uint32_t)new_mcfg;
                            mcfg_replaced=true;
                        }
                        continue;
                    }					
                    if ((!(oem_ssdt)) && table[0]=='S' && table[1]=='S' && table[2]=='D' && table[3]=='T')
                    {
                        DBG("SSDT %d found", curssdt);
                        if (new_ssdts[curssdt])
                        {
                            DBG(" and replaced");
                            xsdt_entries[i-dropoffset]=(uint32_t)new_ssdts[curssdt];
                            totssdt++;
                        }
                        DBG("\n");
                        curssdt++;
                        continue;
                    }					
                    if (table[0]=='D' && table[1]=='S' && table[2]=='D' && table[3]=='T')
                    {
                        DBG("DSDT found\n");

                        xsdt_entries[i-dropoffset]=(uint32_t)new_dsdt;

                        DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);
						
                        continue;
                    }
                    if (table[0]=='F' && table[1]=='A' && table[2]=='C' && table[3]=='P')
                    {
                        struct acpi_2_fadt *fadt, *fadt_mod;
                        fadt=(struct acpi_2_fadt *)(uint32_t)xsdt_entries[i];

                        DBG("FADT found @%x,%x, Length %d\n",(uint32_t)(xsdt_entries[i]>>32),fadt, 
                            fadt->Length);

                        if (!fadt || (uint64_t)xsdt_entries[i] >= 0xffffffff || fadt->Length>0x10000)
                        {
                            verbose("FADT incorrect or after 4GB. Dropping XSDT\n");
                            goto drop_xsdt;
                        }

                        fadt_mod = patch_fadt(fadt, new_dsdt,update_acpi);
                        xsdt_entries[i-dropoffset]=(uint32_t)fadt_mod;

                        DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);

                        continue;
                    }

                    DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);

                }

                if ((!oem_hpet) && (!hpet_replaced))
                {
                    if (new_hpet)
                    {
                        xsdt_entries[i-dropoffset]=(uint32_t)new_hpet;
                        hpet_added=true;
                        i++;
                    }
                }

                if ((!oem_sbst) && (!sbst_replaced))
                {
                    if (new_sbst)
                    {
                        xsdt_entries[i-dropoffset]=(uint32_t)new_sbst;
                        sbst_added=true;
                        i++;
                    }
                }

                if ((!oem_ecdt) && (!ecdt_replaced))
                {
                    if (new_ecdt)
                    {
                        xsdt_entries[i-dropoffset]=(uint32_t)new_ecdt;
                        ecdt_added=true;
                        i++;
                    }
                }

                if ((!oem_asft) && (!asft_replaced))
                {
                    if (new_asft)
                    {
                        xsdt_entries[i-dropoffset]=(uint32_t)new_asft;
                        asft_added=true;
                        i++;
                    }
                }

                if ((!oem_dmar) && (!dmar_replaced))
                {
                    if (new_dmar)
                    {
                        xsdt_entries[i-dropoffset]=(uint32_t)new_dmar;
                        dmar_added=true;
                        i++;
                    }
                }

                if ((!oem_apic) && (!apic_replaced))
                {
                    if (new_apic)
                    {
                        xsdt_entries[i-dropoffset]=(uint32_t)new_apic;
                        apic_added=true;
                        i++;
                    }
                }

                if ((!oem_mcfg) && (!mcfg_replaced))
                {
                    if (new_mcfg)
                    {
                        xsdt_entries[i-dropoffset]=(uint32_t)new_mcfg;
                        mcfg_added=true;
                        i++;
                    }
                }

                if (!oem_ssdt)
                { 			
                    while ((totssdt < loadtotssdt) && (curssdt < 30))
                    {
                        if (new_ssdts[curssdt])
                        {
                            DBG("adding SSDT %d\n", curssdt);
                            xsdt_entries[i-dropoffset]=(uint32_t)new_ssdts[curssdt];
                            totssdt++;
                            newtotssdt++;
                            i++;
                        }
                        curssdt++;
                    }
                }

                // Correct the checksum of XSDT
                xsdt_mod->Length-=8*dropoffset;
                xsdt_mod->Length+=8*newtotssdt;
                if (hpet_added)
                    xsdt_mod->Length+=8;
                if (sbst_added)
                    xsdt_mod->Length+=8;
                if (ecdt_added)
                    xsdt_mod->Length+=8;
                if (asft_added)
                    xsdt_mod->Length+=8;
                if (dmar_added)
                    xsdt_mod->Length+=8;
                if (apic_added)
                    xsdt_mod->Length+=8;
                if (mcfg_added)
                    xsdt_mod->Length+=8;

                xsdt_mod->Checksum=0;
                xsdt_mod->Checksum=256-checksum8(xsdt_mod,xsdt_mod->Length);
            }
            else
            {
            drop_xsdt:

                DBG("About to drop XSDT\n");

                /*FIXME: Now we just hope that if MacOS doesn't find XSDT it reverts to RSDT. 
                 * A Better strategy would be to generate
                 */

                rsdp_mod->XsdtAddress=0xffffffffffffffffLL;
                verbose("XSDT not found or XSDT incorrect\n");
            }
        }

        // Correct the checksum of RSDP      

        DBG("RSDP: Original checksum %d, ", rsdp_mod->Checksum);

        rsdp_mod->Checksum=0;
        rsdp_mod->Checksum=256-checksum8(rsdp_mod,20);

        DBG("New checksum %d\n", rsdp_mod->Checksum);

        if (version)
        {
            DBG("RSDP: Original extended checksum %d", rsdp_mod->ExtendedChecksum);

            rsdp_mod->ExtendedChecksum=0;
            rsdp_mod->ExtendedChecksum=256-checksum8(rsdp_mod,rsdp_mod->Length);

            DBG("New extended checksum %d\n", rsdp_mod->ExtendedChecksum);

        }
		
        verbose("Patched ACPI version %d DSDT\n", version+1);
        if (version)
        {
            /* XXX aserebln why uint32 cast if pointer is uint64 ? */
            acpi20_p = (uint32_t)rsdp_mod;
            addConfigurationTable(&gEfiAcpi20TableGuid, &acpi20_p, "ACPI_20");
        }
        else
        {
            /* XXX aserebln why uint32 cast if pointer is uint64 ? */
            acpi10_p = (uint32_t)rsdp_mod;
            addConfigurationTable(&gEfiAcpiTableGuid, &acpi10_p, "ACPI");
        }
    }
#if DEBUG_DSDT
    printf("Press a key to continue... (DEBUG_DSDT)\n");
    getc();
#endif
    return 1;
}
