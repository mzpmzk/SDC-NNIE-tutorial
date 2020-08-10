//
// Created by liuky on 2020-08-07.
//

#ifndef NNIE_TUTORIAL_SSDMODEL_HPP
#define NNIE_TUTORIAL_SSDMODEL_HPP

#include <hi_type.h>
#include <hi_nnie.h>
#include "algorithm_service.hpp"
#include "utils_service.hpp"


#define SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM    2
/*16Byte align*/
#define SAMPLE_SVP_NNIE_ALIGN_16 16
#define SAMPLE_SVP_NNIE_ALIGN16(u32Num) ((u32Num + SAMPLE_SVP_NNIE_ALIGN_16-1) / SAMPLE_SVP_NNIE_ALIGN_16*SAMPLE_SVP_NNIE_ALIGN_16)
/*32Byte align*/
#define SAMPLE_SVP_NNIE_ALIGN_32 32
#define SAMPLE_SVP_NNIE_ALIGN32(u32Num) ((u32Num + SAMPLE_SVP_NNIE_ALIGN_32-1) / SAMPLE_SVP_NNIE_ALIGN_32*SAMPLE_SVP_NNIE_ALIGN_32)

#define SAMPLE_SVP_NNIE_QUANT_BASE 4096    /*the base value*/
#define SAMPLE_SVP_NNIE_COORDI_NUM  4      /*coordinate numbers*/
#define YUV_CHANNEL_LEN (3)
#define SAMPLE_SVP_NNIE_PROPOSAL_WIDTH  6  /*the number of proposal values*/
#define SAMPLE_SVP_NNIE_MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define SAMPLE_SVP_NNIE_MIN(a,b)    (((a) < (b)) ? (a) : (b))

/*each seg input and output memory*/
typedef struct hiSAMPLE_SVP_NNIE_SEG_DATA_S
{
    SVP_SRC_BLOB_S astSrc[SVP_NNIE_MAX_INPUT_NUM];
    SVP_DST_BLOB_S astDst[SVP_NNIE_MAX_OUTPUT_NUM];
}SVP_NNIE_SEG_DATA_S;

/*NNIE Execution parameters */
typedef struct
{
    SVP_NNIE_MODEL_S*    pstModel;
    HI_U32 u32TmpBufSize;
    HI_U32 au32TaskBufSize[SVP_NNIE_MAX_NET_SEG_NUM];
    SVP_MEM_INFO_S      stStepBuf;//store Lstm step info
    SVP_NNIE_SEG_DATA_S astSegData[SVP_NNIE_MAX_NET_SEG_NUM];//each seg's input and output blob
    SVP_NNIE_FORWARD_CTRL_S astForwardCtrl[SVP_NNIE_MAX_NET_SEG_NUM];
    SVP_NNIE_FORWARD_WITHBBOX_CTRL_S astForwardWithBboxCtrl[SVP_NNIE_MAX_NET_SEG_NUM];
}SVP_NNIE_PARAM_S;

/*SSD software parameter*/
typedef struct
{
    /*----------------- Model Parameters ---------------*/
    HI_U32 au32ConvHeight[12];
    HI_U32 au32ConvWidth[12];
    HI_U32 au32ConvChannel[12];
    /*----------------- PriorBox Parameters ---------------*/
    HI_U32 au32PriorBoxWidth[6];
    HI_U32 au32PriorBoxHeight[6];
    HI_FLOAT af32PriorBoxMinSize[6][1];
    HI_FLOAT af32PriorBoxMaxSize[6][1];
    HI_U32 u32MinSizeNum;
    HI_U32 u32MaxSizeNum;
    HI_U32 u32OriImHeight;
    HI_U32 u32OriImWidth;
    HI_U32 au32InputAspectRatioNum[6];
    HI_FLOAT af32PriorBoxAspectRatio[6][2];
    HI_FLOAT af32PriorBoxStepWidth[6];
    HI_FLOAT af32PriorBoxStepHeight[6];
    HI_FLOAT f32Offset;
    HI_BOOL bFlip;
    HI_BOOL bClip;
    HI_S32 as32PriorBoxVar[4];
    /*----------------- Softmax Parameters ---------------*/
    HI_U32 au32SoftMaxInChn[6];
    HI_U32 u32SoftMaxInHeight;
    HI_U32 u32ConcatNum;
    HI_U32 u32SoftMaxOutWidth;
    HI_U32 u32SoftMaxOutHeight;
    HI_U32 u32SoftMaxOutChn;
    /*----------------- DetectionOut Parameters ---------------*/
    HI_U32 u32ClassNum;
    HI_U32 u32TopK;
    HI_U32 u32KeepTopK;
    HI_U32 u32NmsThresh;
    HI_U32 u32ConfThresh;
    HI_U32 au32DetectInputChn[6];
    HI_U32 au32ConvStride[6];
    SVP_MEM_INFO_S stPriorBoxTmpBuf;
    SVP_MEM_INFO_S stSoftMaxTmpBuf;
    SVP_DST_BLOB_S stClassRoiNum;
    SVP_DST_BLOB_S stDstRoi;
    SVP_DST_BLOB_S stDstScore;
    SVP_MEM_INFO_S stGetResultTmpBuf;
}SVP_NNIE_SSD_SOFTWARE_PARAM_S;

/*NNIE configuration parameter*/
typedef struct
{
    HI_CHAR *pszBGR;/*BGR彩色三通道数据的指针,在模型推断的过程中会将这个指针下的数据拷贝到模型的data blob中作为输入数据*/
    HI_CHAR *pszYUV;/*YUV420SP格式的数据指针*/
    HI_CHAR *pszPic;
//    HI_CHAR **pszBGRs;
    HI_U32 u32MaxInputNum;/*模型的Batch size*/
    HI_U32 u32MaxRoiNum;
    HI_U64 au64StepVirAddr[SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM*SVP_NNIE_MAX_NET_SEG_NUM];//virtual addr of LSTM's or RNN's step buffer
    SVP_NNIE_ID_E	aenNnieCoreId[SVP_NNIE_MAX_NET_SEG_NUM];/*使用的NNIE内核ID*/
}SVP_NNIE_CFG_S;

typedef struct
{
    HI_U32 au32SrcSize[SVP_NNIE_MAX_INPUT_NUM];
    HI_U32 au32DstSize[SVP_NNIE_MAX_OUTPUT_NUM];
}SVP_NNIE_BLOB_SIZE_S;

class SSDModel {
private:
    AlgorithmService *m_algorithm_service;
    UtilsService *m_utils_service;
    SVP_NNIE_PARAM_S nnie_param={0};
    SVP_NNIE_PARAM_S *m_nnie_param = &nnie_param;
    SVP_NNIE_SSD_SOFTWARE_PARAM_S model_software_param={0};
    SVP_NNIE_SSD_SOFTWARE_PARAM_S *m_model_software_param = &model_software_param;
    SVP_NNIE_CFG_S nnie_cfg={0};
    SVP_NNIE_CFG_S *m_nnie_cfg=&nnie_cfg;
    SVP_NNIE_MODEL_WITH_FILE_S *ssd_model;


    static void SVP_NNIE_GetBlobMemSize(SVP_NNIE_NODE_S *astNnieNode, HI_U32 u32NodeNum,
                                        HI_U32 u32TotalStep, SVP_BLOB_S *astBlob, HI_U32 u32Align, HI_U32 *pu32TotalSize,
                                        HI_U32 *au32BlobSize);

    HI_S32 SVP_NNIE_GetTaskAndBlobBufSize(SVP_NNIE_BLOB_SIZE_S astBlobSize[],
                                          HI_U32 *pu32TotalSize);

    HI_S32 SVP_NNIE_FillForwardInfo();

    HI_S32 SVP_NNIE_Ssd_ParamInit();

    HI_S32 SVP_NNIE_Ssd_SoftwareInit();
    HI_S32 SVP_NNIE_Ssd_SoftwareDeinit();

    HI_U32 SVP_NNIE_Ssd_GetResultTmpBuf();

    HI_S32 SVP_NNIE_ParamInit();

    HI_S32 SDC_NNIE_ParamInit();
    HI_S32 SDC_NNIE_ParamDeinit();

    HI_S32 SVP_NNIE_Ssd_Deinit();
public:

    SSDModel(AlgorithmService *algorithm_service , UtilsService *utils_service, SVP_NNIE_MODEL_WITH_FILE_S *model);
    int ssd_param_init(HI_U32 max_input_num,HI_U32 max_roi_num);


};


#endif //NNIE_TUTORIAL_SSDMODEL_HPP
