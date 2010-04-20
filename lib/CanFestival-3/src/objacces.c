/*
  This file is part of CanFestival, a library implementing CanOpen
  Stack.

  Copyright (C): Edouard TISSERANT and Francis DUPIN

  See COPYING file for copyrights details.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
  USA
*/
/*!
** @file   objacces.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Tue Jun  5 08:55:23 2007
**
** @brief
**
**
*/




/* #define DEBUG_WAR_CONSOLE_ON */
/* #define DEBUG_ERR_CONSOLE_ON */


#include "data.h"

#ifdef DEBUG_WAR_CONSOLE_ON
UNS8 accessDictionaryError(UNS16 index, UNS8 subIndex,
                           UNS32 sizeDataDict, UNS32 sizeDataGiven, UNS32 code)
{
  MSG_WAR(0x2B09,"Dictionary index : ", index);
  MSG_WAR(0X2B10,"           subindex : ", subIndex);
  switch (code) {
  case  OD_NO_SUCH_OBJECT:
    MSG_WAR(0x2B11,"Index not found ", index);
    break;
  case OD_NO_SUCH_SUBINDEX :
    MSG_WAR(0x2B12,"SubIndex not found ", subIndex);
    break;
  case OD_WRITE_NOT_ALLOWED :
    MSG_WAR(0x2B13,"Write not allowed, data is read only ", index);
    break;
  case OD_LENGTH_DATA_INVALID :
    MSG_WAR(0x2B14,"Conflict size data. Should be (bytes)  : ", sizeDataDict);
    MSG_WAR(0x2B15,"But you have given the size  : ", sizeDataGiven);
    break;
  case OD_NOT_MAPPABLE :
    MSG_WAR(0x2B16,"Not mappable data in a PDO at index    : ", index);
    break;
  case OD_VALUE_TOO_LOW :
    MSG_WAR(0x2B17,"Value range error : value too low. SDOabort : ", code);
    break;
  case OD_VALUE_TOO_HIGH :
    MSG_WAR(0x2B18,"Value range error : value too high. SDOabort : ", code);
    break;
  default :
    MSG_WAR(0x2B20, "Unknown error code : ", code);
  }
  return 0;
}
#else
#define accessDictionaryError(index, subIndex, sizeDataDict, sizeDataGiven, code)
#endif

UNS32 _getODentry( CO_Data* d,
                   UNS16 wIndex,
                   UNS8 bSubindex,
                   void * pDestData,
                   UNS32 * pExpectedSize,
                   UNS8 * pDataType,
                   UNS8 checkAccess,
                   UNS8 endianize)
{ /* DO NOT USE MSG_ERR because the macro may send a PDO -> infinite
    loop if it fails. */
  UNS32 errorCode;
  UNS32 szData;
  const indextable *ptrTable;
  ODCallback_t *Callback;

  ptrTable = (*d->scanIndexOD)(wIndex, &errorCode, &Callback);

  if (errorCode != OD_SUCCESSFUL)
    return errorCode;
  if( ptrTable->bSubCount <= bSubindex ) {
    /* Subindex not found */
    accessDictionaryError(wIndex, bSubindex, 0, 0, OD_NO_SUCH_SUBINDEX);
    return OD_NO_SUCH_SUBINDEX;
  }

  if (checkAccess && (ptrTable->pSubindex[bSubindex].bAccessType & WO)) {
    MSG_WAR(0x2B30, "Access Type : ", ptrTable->pSubindex[bSubindex].bAccessType);
    accessDictionaryError(wIndex, bSubindex, 0, 0, OD_READ_NOT_ALLOWED);
    return OD_READ_NOT_ALLOWED;
  }

  *pDataType = ptrTable->pSubindex[bSubindex].bDataType;
  szData = ptrTable->pSubindex[bSubindex].size;

  if(*pExpectedSize == 0 ||
     *pExpectedSize == szData ||
     /* allow to fetch a shorter string than expected */
     (*pDataType >= visible_string && *pExpectedSize < szData)) { 

#  ifdef CANOPEN_BIG_ENDIAN
     if(endianize && *pDataType > boolean && !(
            *pDataType >= visible_string && 
            *pDataType <= domain)) {
      /* data must be transmited with low byte first */
      UNS8 i, j = 0;
      MSG_WAR(boolean, "data type ", *pDataType);
      MSG_WAR(visible_string, "data type ", *pDataType);
      for ( i = szData ; i > 0 ; i--) {
        MSG_WAR(i," ", j);
        ((UNS8*)pDestData)[j++] =
          ((UNS8*)ptrTable->pSubindex[bSubindex].pObject)[i-1];
      }
      *pExpectedSize = szData;
    }
    else /* no endianisation change */
#  endif
    if(*pDataType != visible_string) {
        memcpy(pDestData, ptrTable->pSubindex[bSubindex].pObject,szData);
        *pExpectedSize = szData;
    }else{
        /* TODO : CONFORM TO DS-301 : 
         *  - stop using NULL terminated strings
         *  - store string size in td_subindex 
         * */
        /* Copy null terminated string to user, and return discovered size */
        UNS8 *ptr = (UNS8*)ptrTable->pSubindex[bSubindex].pObject;
        UNS8 *ptr_start = ptr;
        /* *pExpectedSize IS < szData . if null, use szData */
        UNS8 *ptr_end = ptr + (*pExpectedSize ? *pExpectedSize : szData) ; 
        UNS8 *ptr_dest = (UNS8*)pDestData;
        while( *ptr && ptr < ptr_end){
            *(ptr_dest++) = *(ptr++);
        } 
         
        *pExpectedSize = ptr - ptr_start;
        /* terminate string if not maximum length */
        if (*pExpectedSize < szData) 
            *(ptr) = 0; 
    }

    return OD_SUCCESSFUL;
  }
  else { /* Error ! */
    *pExpectedSize = szData;
    accessDictionaryError(wIndex, bSubindex, szData,
                          *pExpectedSize, OD_LENGTH_DATA_INVALID);
    return OD_LENGTH_DATA_INVALID;
  }
}

UNS32 _setODentry( CO_Data* d,
                   UNS16 wIndex,
                   UNS8 bSubindex,
                   void * pSourceData,
                   UNS32 * pExpectedSize,
                   UNS8 checkAccess,
                   UNS8 endianize)
{
  UNS32 szData;
  UNS8 dataType;
  UNS32 errorCode;
  const indextable *ptrTable;
  ODCallback_t *Callback;

  ptrTable =(*d->scanIndexOD)(wIndex, &errorCode, &Callback);
  if (errorCode != OD_SUCCESSFUL)
    return errorCode;

  if( ptrTable->bSubCount <= bSubindex ) {
    /* Subindex not found */
    accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_NO_SUCH_SUBINDEX);
    return OD_NO_SUCH_SUBINDEX;
  }
  if (checkAccess && (ptrTable->pSubindex[bSubindex].bAccessType == RO)) {
    MSG_WAR(0x2B25, "Access Type : ", ptrTable->pSubindex[bSubindex].bAccessType);
    accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_WRITE_NOT_ALLOWED);
    return OD_WRITE_NOT_ALLOWED;
  }


  dataType = ptrTable->pSubindex[bSubindex].bDataType;
  szData = ptrTable->pSubindex[bSubindex].size;

  if( *pExpectedSize == 0 ||
      *pExpectedSize == szData ||
      /* allow to store a shorter string than entry size */
      (dataType == visible_string && *pExpectedSize < szData))
    {
#ifdef CANOPEN_BIG_ENDIAN
      /* re-endianize do not occur for bool, strings time and domains */
      if(endianize && dataType > boolean && !(
            dataType >= visible_string && 
            dataType <= domain))
        {
          /* we invert the data source directly. This let us do range
            testing without */
          /* additional temp variable */
          UNS8 i;
          for ( i = 0 ; i < ( ptrTable->pSubindex[bSubindex].size >> 1)  ; i++)
            {
              UNS8 tmp =((UNS8 *)pSourceData) [(ptrTable->pSubindex[bSubindex].size - 1) - i];
              ((UNS8 *)pSourceData) [(ptrTable->pSubindex[bSubindex].size - 1) - i] = ((UNS8 *)pSourceData)[i];
              ((UNS8 *)pSourceData)[i] = tmp;
            }
        }
#endif
      errorCode = (*d->valueRangeTest)(dataType, pSourceData);
      if (errorCode) {
        accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, errorCode);
        return errorCode;
      }
      memcpy(ptrTable->pSubindex[bSubindex].pObject,pSourceData, *pExpectedSize);
     /* TODO : CONFORM TO DS-301 : 
      *  - stop using NULL terminated strings
      *  - store string size in td_subindex 
      * */
      /* terminate visible_string with '\0' */
      if(dataType == visible_string && *pExpectedSize < szData)
        ((UNS8*)ptrTable->pSubindex[bSubindex].pObject)[*pExpectedSize] = 0;
      
      *pExpectedSize = szData;

      /* Callbacks */
      if(Callback && Callback[bSubindex]){
        errorCode = (Callback[bSubindex])(d, ptrTable, bSubindex);
        if(errorCode != OD_SUCCESSFUL)
        {
            return errorCode;
        }
       }

      /* TODO : Store dans NVRAM */
      if (ptrTable->pSubindex[bSubindex].bAccessType & TO_BE_SAVE){
        (*d->storeODSubIndex)(d, wIndex, bSubindex);
      }
      return OD_SUCCESSFUL;
    }else{
      *pExpectedSize = szData;
      accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, OD_LENGTH_DATA_INVALID);
      return OD_LENGTH_DATA_INVALID;
    }
}

const indextable * scanIndexOD (CO_Data* d, UNS16 wIndex, UNS32 *errorCode, ODCallback_t **Callback)
{
  return (*d->scanIndexOD)(wIndex, errorCode, Callback);
}

UNS32 RegisterSetODentryCallBack(CO_Data* d, UNS16 wIndex, UNS8 bSubindex, ODCallback_t Callback)
{
UNS32 errorCode;
ODCallback_t *CallbackList;
const indextable *odentry;

  odentry = scanIndexOD (d, wIndex, &errorCode, &CallbackList);
  if(errorCode == OD_SUCCESSFUL  &&  CallbackList  &&  bSubindex < odentry->bSubCount) 
    CallbackList[bSubindex] = Callback;
  return errorCode;
}

void _storeODSubIndex (CO_Data* d, UNS16 wIndex, UNS8 bSubindex){}
