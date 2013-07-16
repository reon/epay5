/******************************************************************
** Copyright(C)2006 - 2008���������豸���޹�˾
** ��Ҫ����:����ʹ�õ�С���ܻ��ӿں���
           
** �� �� ��:Robin 
** ��������:2009/08/29


$Revision: 1.11 $
$Log: Sjl06s.c,v $
Revision 1.11  2012/12/26 01:44:17  wukj
%s/commu_with_hsm/CommuWithHsm/g

Revision 1.10  2012/12/05 06:32:14  wukj
*** empty log message ***

Revision 1.9  2012/11/29 07:51:43  wukj
�޸���־����,�޸�ascbcdת������

Revision 1.8  2012/11/29 01:57:55  wukj
��־�����޸�

Revision 1.7  2012/11/21 04:13:38  wukj
�޸�hsmincl.h Ϊhsm.h

Revision 1.6  2012/11/21 03:20:31  wukj
1:���ܻ����������޸� 2: ȫ�ֱ�������hsmincl.h


*******************************************************************/

#include "hsm.h"

/*****************************************************************
** ��    ��:���ܻ���������ն�����ԿTMK�����������ĺͼ��ֵ���ظ�������
** �������:
           ��
** �������:
           tInterface->szData  TMK(�������32Bytes)+TMK(�ַ����ն�32Bytes)+CheckValue(4Bytes)
** �� �� ֵ:
           �ɹ� - SUCC
           ʧ�� - FAIL
** ��    ��:Robin     
** ��    ��:2009/08/25 
** ����˵��:
** �޸���־:mod by wukj 20121031�淶�������Ű��޶�
**          
****************************************************************/
int Sjl06sGetTmk( T_Interface *tInterface, int iSekTmkIndex, int iTekIndex )
{
    char    szInData[1024], szOutData[1024], szRetCode[3];
    int     iLen, iRet, iSndLen, i;

    iLen = 0;
    /*����*/
    memcpy( szInData, "11", 2 );    
    iLen += 2;
    /* sek */
    //sprintf( szInData+iLen, "%03ld", iSekTmkIndex ); 
    sprintf( szInData+iLen, "%03ld", iTekIndex ); 
    iLen += 3;
    szInData[iLen] = 0;
    memset( szOutData, 0, 1024 );
    iRet = CommuWithHsm( szInData, iLen, szOutData ); 
    if( iRet == FAIL )
    {
        WriteLog( ERROR, "commu with hsm fail" );
        return FAIL;
    }
    /* ���ܻ����ش��� */
    if( memcmp(szOutData, "E", 1) == 0 )
    {
        DispUphsmErrorMsg( szOutData+1, tInterface->szReturnCode );
        WriteLog( ERROR, "hsm fail[%2.2s]", szOutData+1 );
        return SUCC;
    }
    else if( memcmp(szOutData, "A", 1) == 0 )
    {
        /* SEK���ܵ���Կ */
        memcpy( tInterface->szData, szOutData+1, 32 );
        memcpy( tInterface->szData+32, szOutData+1, 32 );
        /* ��ԿУ��ֵ */
        /*memcpy( tInterface->szData+64, "0000", 4 );*/
        memcpy( tInterface->szData+64, szOutData+1+32, 4 );
        tInterface->iDataLen = 68;
        strcpy( tInterface->szReturnCode, TRANS_SUCC );
    }

    return SUCC;
}

/*****************************************************************
** ��    ��:����������ԿPIK/MAK�����������ĺͼ��ֵ���ظ�������
** �������:
           tInterface->szData �ն�����Կ����(32)
           iTempBMKIndex     ָ������������Կ����,���ڼ��������ն˹�����Կ
           iSekWorkIndex     ��������,���ڼ��ܹ�����Կ
** �������:
           tInterface->szData  PIK(�������32Bytes)+PIK(�´�POS32Bytes)+CheckValue(16Bytes)+
                             MAK(�������32Bytes)+PIK(�´�POS32Bytes)+CheckValue(16Bytes)+
                             MAG(�������32Bytes)+PIK(�´�POS32Bytes)+CheckValue(16Bytes)
** �� �� ֵ:
           �ɹ� - SUCC
           ʧ�� - FAIL
** ��    ��:Robin     
** ��    ��:2009/08/25 
** ����˵��:
** �޸���־:mod by wukj 20121031�淶�������Ű��޶�
**          
****************************************************************/
int Sjl06sGetWorkKey(T_Interface *tInterface, int iSekTmkIndex,
        int iSekWorkIndex, int iTempBMKIndex )
{
    char    szInData[1024], szOutData[1024], szPsamNo[17], szRand[33];
    char    szEnTmk[33], szChkVal[17];
    int     iLen, iRet, iSndLen, i;
    char    cChr;

    memcpy( szEnTmk, tInterface->szData, 32 );

    /* ���ն�����Կ�������������Կ�����ָ�����������ڼ��������ն˹�����Կ */
    iLen = 0;
    /*����*/
    memcpy( szInData, "20", 2 );    
    iLen += 2;
    /* ָ����������Կ���� */
    sprintf( szInData+iLen, "%03ld", iTempBMKIndex ); 
    iLen += 3;

    /*�ն�����Կ����*/
    memcpy( szInData+iLen, szEnTmk, 32 );
    iLen += 32;
    szInData[iLen] = 0;
    memset( szOutData, 0, 1024 );
    iRet = CommuWithHsm( szInData, iLen, szOutData ); 
    if( iRet == FAIL )
    {
        WriteLog( ERROR, "commu with hsm fail" );
        return FAIL;
    }
    if( memcmp(szOutData, "E", 1) == 0 )
    {
        DispUphsmErrorMsg( szOutData+1, tInterface->szReturnCode );
        WriteLog( ERROR, "hsm fail[%2.2s]", szOutData+1 );
        return SUCC;
    }
    else if( memcmp(szOutData, "A", 1) == 0 )
    {
         /* ����������Կ��У��ֵ */
        for( i=0; i<3; i++ )
        {
            /*==================�������ն�����Կ���ܵĹ�����Կ===================*/
            iLen = 0;
            /*����*/
            memcpy( szInData, "XE", 2 );
            iLen += 2;
            /*��������Կ����*/
            sprintf( szInData+iLen, "%03ld", iTempBMKIndex );
            iLen += 3;
            szInData[iLen] = 0x00;
            memset( szOutData, 0, 1024 );
            iRet = CommuWithHsm( szInData, iLen, szOutData );
            if( iRet == FAIL )
            {
                WriteLog( ERROR, "commu with hsm fail" );
                strcpy( tInterface->szReturnCode, ERR_SYSTEM_ERROR );
                return FAIL;
            }
            /* ���ܻ����ش��� */
            if( memcmp(szOutData, "E", 1) == 0 )
            {
                DispUphsmErrorMsg( szOutData+1, tInterface->szReturnCode );
                WriteLog( ERROR, "hsm fail[%2.2s]", szOutData+1 );
                return SUCC;
            }
            else if( memcmp(szOutData, "A", 1) == 0 )
            {
                /* SEK���ܵ���Կ */
                memcpy( tInterface->szData+i*80+32, szOutData+1, 32 );
                memcpy( szChkVal, szOutData+33, 16 );
                /*==============������Կ���ն�����Կ����ת���ɱ�������Կ����(�̶�����)==============*/
                iLen = 0;
                /*����*/
                memcpy( szInData, "XF", 2 );
                iLen += 2;
                /*��������Կ1����*/
                sprintf( szInData+iLen, "%03ld", iTempBMKIndex );
                iLen += 3;
                /*��������Կ2���� --- ������Կ������Կ���� */
                sprintf( szInData+iLen, "%03ld", iSekWorkIndex );
                iLen += 3;
                /*������Կ����(�ն�����Կ����)*/
                memcpy( szInData+iLen, szOutData+1, 32 );
                iLen += 32;
                szInData[iLen] = 0;
                memset( szOutData, 0, 1024 );
                iRet = CommuWithHsm( szInData, iLen, szOutData );
                if( iRet == FAIL )
                {
                    WriteLog( ERROR, "commu with hsm fail" );
                    strcpy( tInterface->szReturnCode, ERR_SYSTEM_ERROR );
                    return FAIL;
                }
                if( memcmp(szOutData, "E", 1) == 0 )
                {
                    DispUphsmErrorMsg( szOutData+1, tInterface->szReturnCode );
                    WriteLog( ERROR, "hsm fail[%2.2s]", szOutData+1 );
                    return SUCC;
                }
                else if( memcmp(szOutData, "A", 1) == 0 )
                {
                    memcpy( tInterface->szData+i*80, szOutData+1, 32 );
                    memcpy( tInterface->szData+i*80+64, szChkVal, 16 );
                }
            }
        }    
        tInterface->iDataLen = 240;
        
        strcpy( tInterface->szReturnCode, TRANS_SUCC );
    }
   
    return SUCC;
}

/*****************************************************************
** ��    ��:��ANSI X9.9 MAC�㷨��������MAC�� 
** �������:
           tInterface->szData ����MAC��������ݣ�������data_lenָ��
           iSekIndex        �洢������Կ����
** �������:
           tInterface->szData MAC(8�ֽ�)
** �� �� ֵ:
           �ɹ� - SUCC
           ʧ�� - FAIL
** ��    ��:Robin     
** ��    ��:2009/08/25 
** ����˵��:
** �޸���־:mod by wukj 20121031�淶�������Ű��޶�
**          
****************************************************************/
int Sjl06sCalcMac(T_Interface *tInterface, int iSekIndex)
{
    char    szInData[2048], szOutData[2048], szMacData[17];
    int     iLen, iRet, iSndLen;
    char    szDisp[1024];

    iLen = 0;
    /* ���� */
    memcpy( szInData, "XR", 2 );    
    iLen += 2;
    /*�洢������Կ����*/
    sprintf( szInData+iLen, "%03ld", iSekIndex ); 
    iLen += 3;

    /*MAC��Կ����*/
    BcdToAsc( (uchar *)(tInterface->szMacKey), 32, 0 ,(uchar *)(szInData+iLen));
    iLen += 32;

#ifdef TEST_SAFE_PRT
    memset(szDisp, 0x00, sizeof(szDisp));
    BcdToAsc((uchar*)(tInterface->szMacKey), 32, 0,(uchar*)szDisp);
    WriteLog(TRACE, "===mac key[%s]===", szDisp);
#endif
    
    if( tInterface->iAlog == XOR_CALC_MAC )
    {
        XOR( tInterface->szData, tInterface->iDataLen, szMacData );
        sprintf( szInData+iLen, "%03ld", 8 );
        iLen += 3;

        memcpy( szInData+iLen, szMacData, 8 );
        iLen += 8;
    }
    else
    {
        sprintf( szInData+iLen, "%03ld", tInterface->iDataLen );
        iLen += 3;

        memcpy( szInData+iLen, tInterface->szData, tInterface->iDataLen );
        iLen += tInterface->iDataLen;
    }
    
    iRet = CommuWithHsm( szInData, iLen, szOutData ); 
    if( iRet == FAIL )
    {
        WriteLog( ERROR, "commu with hsm fail" );
        strcpy( tInterface->szReturnCode, ERR_SYSTEM_ERROR );
        return FAIL;
    }

    if( memcmp(szOutData, "E", 1) == 0 )
    {
        DispUphsmErrorMsg( szOutData+1, tInterface->szReturnCode );
        WriteLog( ERROR, "hsm calc mac fail[%2.2s]", szOutData+1 );
        return SUCC;
    }
    else if(memcmp(szOutData, "A", 1) == 0 )
    {
        AscToBcd( (uchar *)(szOutData+1), 16, 0 ,(uchar *)(tInterface->szData));
        tInterface->iDataLen = 8;
        strcpy( tInterface->szReturnCode, TRANS_SUCC );
    }
   
    return SUCC;
}

/*****************************************************************
** ��    ��:��ԴPIN������ԴPIK���ܣ�����PIN��ʽת����Ȼ����Ŀ��PIK���������
** �������:
           tInterface->szData Դ�ʺ�(16�ֽ�)+��������(8�ֽ�)+Ŀ���ʺ�(16�ֽ�)
           iSekPosIndex     ����ԴPIK�Ĵ洢������Կ����
           iSekHostIndex    ����Ŀ��PIK�Ĵ洢������Կ����
** �������:
           tInterface->szData ת���ܺ����������(8�ֽ�)
** �� �� ֵ:
           �ɹ� - SUCC
           ʧ�� - FAIL
** ��    ��:Robin     
** ��    ��:2009/08/25 
** ����˵��:
** �޸���־:mod by wukj 20121031�淶�������Ű��޶�
**          
****************************************************************/
int Sjl06sChangePin(T_Interface *tInterface, int iSekPosIndex, int iSekHostIndex)
{
    char    szInData[1024], szOutData[1024], szPanBlock[17], szTargetPan[17];
    int     iLen, iRet, iSndLen;

    sprintf( szPanBlock, "0000%12.12s", tInterface->szData+3 );
    szPanBlock[16] = 0;

    sprintf( szTargetPan, "0000%12.12s", tInterface->szData+27 );
    szTargetPan[16] = 0;

    iLen = 0;
    memcpy( szInData, "XP", 2 );    /* ���� */
    iLen += 2;

    /* ����ԴPIK�Ĵ洢������Կ���� */
    sprintf( szInData+iLen, "%03ld", iSekPosIndex );
    iLen += 3;

    /* ԴPIK��Կ���� */
    BcdToAsc( (uchar *)(tInterface->szPinKey), 32, 0 , (uchar *)(szInData+iLen));    
    iLen += 32;

    /* ����Ŀ��PIK�Ĵ洢������Կ���� */
    sprintf( szInData+iLen, "%03ld", iSekHostIndex );
    iLen += 3;

    /* Ŀ��PIK��Կ���� */
    BcdToAsc( (uchar *)(tInterface->szMacKey), 32, 0 , (uchar *)(szInData+iLen));    
    iLen += 32;

    /* ԴPinBlock���� */
    BcdToAsc( (uchar *)(tInterface->szData+16), 16, 0 , (uchar *)(szInData+iLen));    
    iLen += 16;

    /* Դ�ʺ� */
    memcpy( szInData+iLen, szPanBlock, 16 );    
    iLen += 16;

    /* Ŀ���ʺ� */
    memcpy( szInData+iLen, szTargetPan, 16 );    
    iLen += 16;

    iRet = CommuWithHsm( szInData, iLen, szOutData ); 
    if( iRet == FAIL )
    {
        WriteLog( ERROR, "commu with hsm fail" );
        strcpy( tInterface->szReturnCode, ERR_SYSTEM_ERROR );
        return FAIL;
    }

    if( memcmp(szOutData, "E", 1) == 0 )
    {
        DispUphsmErrorMsg( szOutData+1, tInterface->szReturnCode );
        WriteLog( ERROR, "hsm pin change fail[%2.2s]", szOutData+1 );
        return SUCC;
    }
    else if(memcmp(szOutData, "A", 1) == 0 )
    {
        AscToBcd( (uchar *)(szOutData+1), 16, 0 ,(uchar *)(tInterface->szData));
        tInterface->iDataLen = 8;
        strcpy( tInterface->szReturnCode, TRANS_SUCC );
    }
   
    return SUCC;
}

/*****************************************************************
** ��    ��:��֤�ն����͵�PIN�Ƿ������ݿ��е�PINһ��
            ��2��:
            1����PIK��PIN���ļ���
            2�����ն����͵����Ľ��бȽ�
** �������:
           tInterface->szData ���ݿ�����������(8�ֽ�)+�ն�PIN����(8�ֽ�)
           iSekPosIndex     ����PIK�Ĵ洢������Կ����
** �������:
           tInterface->szData SUCC-һ��  FAIL-��һ��
** �� �� ֵ:
           �ɹ� - SUCC
           ʧ�� - FAIL
** ��    ��:Robin     
** ��    ��:2009/08/25 
** ����˵��:
** �޸���־:mod by wukj 20121031�淶�������Ű��޶�
**          
****************************************************************/
int Sjl06sVerifyPin(T_Interface *tInterface, int iSekPosIndex)
{
    char    szInData[1024], szOutData[1024], szPanBlock[17], szEncPin[17];
    int     iLen, iRet, iSndLen;

    memset( szPanBlock, '0', 16 );
    szPanBlock[16] = 0;

    iLen = 0;
    memcpy( szInData, "XI", 2 );    /* ���� */
    iLen += 2;

    /*PIN���� ANSIX9.8�㷨*/ 
    memcpy( szInData+iLen, "3", 1 );    
    iLen += 1;

    /* ����PIK�Ĵ洢������Կ���� */
    sprintf( szInData+iLen, "%03ld", iSekPosIndex );
    iLen += 3;

    /* PIK��Կ���� */
    BcdToAsc( (uchar *)(tInterface->szPinKey), 32, 0 , (uchar *)(szInData+iLen));    
    iLen += 32;

    /* PIN���� */
    memcpy( szInData+iLen, "08", 2 );
    iLen += 2;
    memcpy( szInData+iLen, tInterface->szData, 8 );
    iLen += 8;
    memcpy( szInData+iLen, "FFFFFFFF", 6 );
    iLen += 6;

    /* �ʺ� */
    memcpy( szInData+iLen, szPanBlock, 16 );    
    iLen += 16;

    iRet = CommuWithHsm( szInData, iLen, szOutData ); 
    if( iRet == FAIL )
    {
        WriteLog( ERROR, "commu with hsm fail" );
        strcpy( tInterface->szReturnCode, ERR_SYSTEM_ERROR );
        return FAIL;
    }

    if( memcmp(szOutData, "E", 1) == 0 )
    {
        DispUphsmErrorMsg( szOutData+1, tInterface->szReturnCode );
        WriteLog( ERROR, "hsm encrypt pin fail[%2.2s]", szOutData+1 );
        return SUCC;
    }
    else if(memcmp(szOutData, "A", 1) == 0 )
    {
        AscToBcd( (uchar *)(szOutData+1), 16, 0 ,(uchar *)szEncPin);
        if( memcmp( tInterface->szData+8, szEncPin, 8 ) == 0 )
        {
            strcpy( tInterface->szData, "SUCC" );
        }
        else
        {
            strcpy( tInterface->szData, "FAIL" );
        }
        tInterface->iDataLen = 4;
        strcpy( tInterface->szReturnCode, TRANS_SUCC );
    }
   
    return SUCC;
}

/*****************************************************************
** ��    ��:������Կ��У��ֵ
** �������:
           tInterface->szData ��Կ����(32�ֽ�)
           iSekIndex        ����PIK�Ĵ洢������Կ����
** �������:
           tInterface->szData У��ֵ(16)
** �� �� ֵ:
           �ɹ� - SUCC
           ʧ�� - FAIL
** ��    ��:Robin     
** ��    ��:2009/08/25 
** ����˵��:
** �޸���־:mod by wukj 20121031�淶�������Ű��޶�
**          
****************************************************************/
int Sjl06sCalcChkval(T_Interface *tInterface, int iSekIndex)
{
    char    szInData[1024], szOutData[1024], szPanBlock[17];
    int     iLen, iRet, iSndLen;

    iLen = 0;
    memcpy( szInData, "XC", 2 );    /* ���� */
    iLen += 2;

    /* ����PIK�Ĵ洢������Կ���� */
    sprintf( szInData+iLen, "%03ld", iSekIndex );
    iLen += 3;

    /* ��Կ���� */
    memcpy( szInData+iLen, tInterface->szData, 32 );
    iLen += 32;

    iRet = CommuWithHsm( szInData, iLen, szOutData ); 
    if( iRet == FAIL )
    {
        WriteLog( ERROR, "commu with hsm fail" );
        strcpy( tInterface->szReturnCode, ERR_SYSTEM_ERROR );
        return FAIL;
    }

    if( memcmp(szOutData, "E", 1) == 0 )
    {
        DispUphsmErrorMsg( szOutData+1, tInterface->szReturnCode );
        WriteLog( ERROR, "hsm encrypt pin fail[%2.2s]", szOutData+1 );
        return SUCC;
    }
    else if(memcmp(szOutData, "A", 1) == 0 )
    {
        memcpy( tInterface->szData, szOutData+1, 4 );
        memcpy( tInterface->szData+4, "0000", 4 );
        tInterface->iDataLen = 8;
        strcpy( tInterface->szReturnCode, TRANS_SUCC );
    }
   
    return SUCC;
}

/*****************************************************************
** ��    ��:���ܻ�ת���ܹ�����Կ��������TMK���ܵĹ�����Կ��ԭ�����ģ�Ȼ������ָ����SEK����
** �������:
           tInterface->szData       TMK����(32Bytes)+PIK����(32Bytes)+MAK����(32Bytes)��������Կ��TMK����
           iSekTmkIndex           ����TMK��SEK��Կ����
           iSekWorkIndex       ���ܹ�����Կ��SEK��Կ����
** �������:
           tInterface->szData       PIK����(32Bytes)+MAK����(32Bytes)+TMK����(32Bytes)��������Կ��ָ��SEK����
** �� �� ֵ:
           �ɹ� - SUCC
           ʧ�� - FAIL
** ��    ��:Robin     
** ��    ��:2009/08/25 
** ����˵��:
** �޸���־:mod by wukj 20121031�淶�������Ű��޶�
**          
****************************************************************/
int Sjl06sChangeWorkKey( T_Interface *tInterface, int iSekTmkIndex, int iSekWorkIndex, int iTempBMKIndex )
{
    char    szInData[1024], szOutData[1024], szRetCode[3], szTmk[33];
    char    szOutKey[256];
    int     iLen, iRet, iSndLen, i;

    memcpy( szTmk, tInterface->szData, 32 );

    /* ���ն�����Կ�������������Կ�����ָ������(������������Կ1ʹ��)������ת�����ն˹�����Կ */
    iLen = 0;
    /*����*/
    memcpy( szInData, "20", 2 );    
    iLen += 2;
    /* ָ����������Կ���� */
    sprintf( szInData+iLen, "%03ld", iTempBMKIndex ); 
    iLen += 3;
    /*�ն�����Կ����*/
    memcpy( szInData+iLen, szTmk, 32 );
    iLen += 32;
    szInData[iLen] = 0;
    memset( szOutData, 0, 1024 );
    iRet = CommuWithHsm( szInData, iLen, szOutData ); 
    if( iRet == FAIL )
    {
        WriteLog( ERROR, "commu with hsm fail" );
        return FAIL;
    }
    if( memcmp(szOutData, "E", 1) == 0 )
    {
        DispUphsmErrorMsg( szOutData+1, tInterface->szReturnCode );
        WriteLog( ERROR, "hsm fail[%2.2s]", szOutData+1 );
        return SUCC;
    }
    else if(memcmp(szOutData, "A", 1) == 0 )
    {
        for( i=1; i<=2; i++ )
        {
            iLen = 0;
            /*����*/
            memcpy( szInData, "XF", 2 );    
            iLen += 2;
            /* sek1 */
            sprintf( szInData+iLen, "%03ld", iTempBMKIndex ); 
            iLen += 3;
            /* sek2 */
            sprintf( szInData+iLen, "%03ld", iSekWorkIndex); 
            iLen += 3;
            /* PIK/MAC���� */
            memcpy( szInData+iLen, tInterface->szData+32*i, 32 );
            iLen += 32;
            szInData[iLen] = 0;
            memset( szOutData, 0, 1024 );
            iRet = CommuWithHsm( szInData, iLen, szOutData ); 
            if( iRet == FAIL )
            {
                WriteLog( ERROR, "commu with hsm fail" );
                return FAIL;
            }
            if( memcmp(szOutData, "E", 1) == 0 )
            {
                DispUphsmErrorMsg( szOutData+1, tInterface->szReturnCode );
                WriteLog( ERROR, "hsm fail[%2.2s]", szOutData+1 );
                return SUCC;
            }
            else if( memcmp(szOutData, "A", 1) == 0 )
            {
                memcpy( szOutKey+32*(i-1), szOutData+1, 32 );
            }
        }
        memcpy( tInterface->szData, szOutKey, 64 );
        memcpy( tInterface->szData+64, szTmk, 32 );
        tInterface->iDataLen = 96;
        strcpy( tInterface->szReturnCode, TRANS_SUCC );
    }

    return SUCC;
}

/*****************************************************************
** ��    ��:�洢һ����������������Կ
** �������:
            iKeyIndex   �洢����
            szEnBMK     ��Կֵ
** �������:
            ��
** �� �� ֵ:
           �ɹ� - SUCC
           ʧ�� - FAIL
** ��    ��:Robin     
** ��    ��:2009/08/25 
** ����˵��:
** �޸���־:mod by wukj 20121031�淶�������Ű��޶�
**          
****************************************************************/
int Sjl06sSaveBMK( int iKeyIndex, char *szEnBMK )
{
    char    szInData[1024], szOutData[1024];
    int     iLen, iRet;

    /* ���ն�����Կ�������������Կ�����ָ�����������ڼ��������ն˹�����Կ */
    iLen = 0;
    /*����*/
    memcpy( szInData, "20", 2 );    
    iLen += 2;
    /* ָ����������Կ���� */
    sprintf( szInData+iLen, "%03ld", iKeyIndex ); 
    iLen += 3;

    /*�ն�����Կ����*/
    memcpy( szInData+iLen, szEnBMK, 32 );
    iLen += 32;
    szInData[iLen] = 0;
    memset( szOutData, 0, 1024 );
    iRet = CommuWithHsm( szInData, iLen, szOutData ); 
    if( iRet == FAIL )
    {
        WriteLog( ERROR, "commu with hsm fail" );
        return FAIL;
    }
    if( memcmp(szOutData, "E", 1) == 0 )
    {
        WriteLog( ERROR, "hsm fail[%2.2s]", szOutData+1 );
        return SUCC;
    }
    else if( memcmp(szOutData, "A", 1) == 0 ) 
    {
        return SUCC;
    }
}