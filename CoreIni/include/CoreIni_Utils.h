//~---------------------------------------------------------------------------//
//                     _______  _______  _______  _     _                     //
//                    |   _   ||       ||       || | _ | |                    //
//                    |  |_|  ||       ||   _   || || || |                    //
//                    |       ||       ||  | |  ||       |                    //
//                    |       ||      _||  |_|  ||       |                    //
//                    |   _   ||     |_ |       ||   _   |                    //
//                    |__| |__||_______||_______||__| |__|                    //
//                             www.amazingcow.com                             //
//  File      : CoreIni_Utils.h                                               //
//  Project   : CoreIni                                                       //
//  Date      : Dec 23, 2017                                                  //
//  License   : GPLv3                                                         //
//  Author    : n2omatt <n2omatt@amazingcow.com>                              //
//  Copyright : AmazingCow - 2017, 2018                                       //
//                                                                            //
//  Description :                                                             //
//---------------------------------------------------------------------------~//

#pragma once

//----------------------------------------------------------------------------//
// Namespaces                                                                 //
//----------------------------------------------------------------------------//
// All classes of this core is placed inside this namespace.
// We use MACROS so is easier to change if needed.
// Is (in our opinion) more explicit.
// And finally the editors will not reformat the code.
#define NS_COREINI_BEGIN namespace CoreIni {
#define NS_COREINI_END   }
#define USING_NS_COREINI using namespace CoreIni


//----------------------------------------------------------------------------//
// Version                                                                    //
//----------------------------------------------------------------------------//
#define COW_COREINI_VERSION_MAJOR    "0"
#define COW_COREINI_VERSION_MINOR    "1"
#define COW_COREINI_VERSION_REVISION "0"

#define COW_COREINI_VERSION           \
        COW_COREINI_VERSION_MAJOR "." \
        COW_COREINI_VERSION_MINOR "." \
        COW_COREINI_VERSION_REVISION
