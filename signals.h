#ifndef __SIGNALS_H__
#define __SIGNALS_H__

/*=============================================================================
* includes, defines, usings
=============================================================================*/

/*=============================================================================
* global functions
=============================================================================*/
void handleFinishChld(int sig);
void handle_ctrl_c(int sig);
void handle_ctrl_z(int sig);
void MainHandleConfigPack();


#endif //__SIGNALS_H__