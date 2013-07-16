/******************************************************************
** Copyright(C)2006 - 2008联迪商用设备有限公司
** 主要内容：易收付平台金融交易处理模块 获取机构参数定义
** 创 建 人：冯炜
** 创建日期：2012-11-08
**
** $Revision: 1.2 $
** $Log: GetDeptConf.ec,v $
** Revision 1.2  2012/12/04 01:24:28  fengw
**
** 1、替换ErrorLog为WriteLog。
**
** Revision 1.1  2012/11/23 09:09:16  fengw
**
** 金融交易处理模块初始版本
**
** Revision 1.2  2012/11/22 09:00:39  fengw
**
** 1、修正机构参数配置查询SQL语句
**
** Revision 1.1  2012/11/21 07:20:46  fengw
**
** 金融交易处理模块初始版本
**
*******************************************************************/

#define _EXTERN_

#include "finatran.h"

EXEC SQL BEGIN DECLARE SECTION;
    EXEC SQL INCLUDE SQLCA;
EXEC SQL END DECLARE SECTION;

/****************************************************************
** 功    能：获取机构参数定义
** 输入参数：
**        iTransType           交易类型
**        szDeptDetail         机构层级信息
** 输出参数：
**        ptEpayConf           参数定义
** 返 回 值：
**        CONF_GET_SUCC        参数查询成功
**        CONF_GET_FAIL        参数查询失败
**        CONF_NOT_FOUND       未定义参数
** 作    者：
**        fengwei
** 日    期：
**        2012/11/08
** 调用说明：
**
** 修改日志：
****************************************************************/
int GetDeptConf(int iTransType, char *szDeptDetail, T_EpayConf *ptEpayConf)
{
    EXEC SQL BEGIN DECLARE SECTION;
        int     iHostType;                          /* 交易类型 */
        char    szHostDeptDetail[70+1];             /* 机构层级信息 */
        double  dAmountSingle;                      /* 单笔限额 */
        double  dAmountSum;                         /* 当日累计限额 */
        int     iMaxCount;                          /* 当日最大交易笔数 */
        double  dCreditAmountSingle;                /* 信用卡单笔限额 */
        double  dCreditAmountSum;                   /* 信用卡当日累计限额 */
        int     iCreditMaxCount;                    /* 信用卡当日最大交易笔数 */
        char    szCardTypeOut[9+1];                 /* 转出卡许可卡类型 */
        char    szCardTypeIn[9+1];                  /* 转入卡许可卡类型 */
        int     iFeeCalcType;                       /* 手续费计算方式 */
    EXEC SQL END DECLARE SECTION;

    memset(szHostDeptDetail, 0, sizeof(szHostDeptDetail));
    strcpy(szHostDeptDetail, szDeptDetail);

    iHostType = iTransType;

    memset(szCardTypeOut, 0, sizeof(szCardTypeOut));
    memset(szCardTypeIn, 0, sizeof(szCardTypeIn));

    EXEC SQL
        SELECT amount_single, amount_sum, max_count,
               credit_amount_single, credit_amount_sum, credit_max_count,
               card_type_out, card_type_in, fee_calc_type
        INTO :dAmountSingle, :dAmountSum, :iMaxCount,
             :dCreditAmountSingle, :dCreditAmountSum, :iCreditMaxCount,
             :szCardTypeOut, :szCardTypeIn, :iFeeCalcType
        FROM
            (SELECT amount_single, amount_sum, max_count,
                    credit_amount_single, credit_amount_sum, credit_max_count,
                    card_type_out, card_type_in, fee_calc_type
            FROM dept_conf
            WHERE trans_type = :iHostType AND
            INSTR(:szHostDeptDetail, dept_detail) = 1
            ORDER BY LENGTH(dept_detail) DESC) WHERE ROWNUM = 1;
    if(SQLCODE == SQL_NO_RECORD)
    {
        return CONF_NOT_FOUND;
    }
    else if(SQLCODE)
    {
        WriteLog(ERROR, "机构[%s] 交易类型[%d] 查询参数失败!SQLCODE=%d SQLERR=%s",
                 szHostDeptDetail, iHostType, SQLCODE, SQLERR);

        return CONF_GET_FAIL;
    }

    /* 参数赋值 */
    ptEpayConf->dAmountSingle = dAmountSingle;
    ptEpayConf->dAmountSum = dAmountSum;
    ptEpayConf->iMaxCount = iMaxCount;
    ptEpayConf->dCreditAmountSingle = dCreditAmountSingle;
    ptEpayConf->dCreditAmountSum = dCreditAmountSum;
    ptEpayConf->iCreditMaxCount = iCreditMaxCount;
    memcpy(ptEpayConf->szCardTypeOut, szCardTypeOut, strlen(szCardTypeOut));
    memcpy(ptEpayConf->szCardTypeIn, szCardTypeIn, strlen(szCardTypeIn));
    ptEpayConf->iFeeCalcType = iFeeCalcType;

    return CONF_GET_SUCC;
}