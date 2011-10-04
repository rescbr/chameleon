

#ifndef __INTERNAL_MODULES_H
#define __INTERNAL_MODULES_H



static inline void load_all_internal_modules()
{
  // HOW TO: the procedure is quite simple but not automatic yet,
    
  // to embed a module just drag/copy the content your module folder (*.c,*.h, etc...)
  // into libsaio folder, edit the libsaio makefile to build the files that you just added,
  // and add here some code to start the module,
    
  // ex. :
    
   //extern void ACPICodec_start();
   //ACPICodec_start();
    
   //extern void SMBiosGetters_start();
   //SMBiosGetters_start();
    
    //extern void GUI_start();
    //GUI_start();

   // Done !!!
    
   // ps: i swear the next version will be more simple 
}

#endif /* __INTERNAL_MODULES_H */