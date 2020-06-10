#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "plzprintmeta.h"

GST_DEBUG_CATEGORY_STATIC (gst_plzprintmeta_debug);
#define GST_CAT_DEFAULT gst_plzprintmeta_debug

static GQuark _dsmeta_quark = 0;

const gchar sgie_classes_str[23][32] = {
  "023100014074", "3320", "4011", "4381", "4408", "4593", "4664", "93283", "94016",
  "94020", "94053", "94068", "94295", "beetroot", "broccoli", "brusselsprout",
  "chineseeggplant", "corn", "garlic", "greenbeans", "jalepeno", "serrano", "turnip"
};

#define GST_CAPS_FEATURE_MEMORY_NVMM "memory:NVMM"
static GstStaticPadTemplate sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES
        (GST_CAPS_FEATURE_MEMORY_NVMM,
            "{ NV12, RGBA, I420 }")));

static GstStaticPadTemplate src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES
        (GST_CAPS_FEATURE_MEMORY_NVMM,
            "{ NV12, RGBA, I420 }")));

G_DEFINE_TYPE (GstPlzPrintMeta, gst_plzprintmeta, GST_TYPE_BASE_TRANSFORM);


static void gst_plzprintmeta_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_plzprintmeta_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static GstFlowReturn gst_plzprintmeta_transform_ip (GstBaseTransform * btrans,
    GstBuffer * buffer);
static gboolean gst_plzprintmeta_stop (GstBaseTransform * btrans);

#define PLZPRINTMETA_NAME "plzprintmeta"
#define PLZPRINTMETA_VERSION "0.1.0"
#define PLZPRINTMETA_LONG_NAME "Printing out labels and metadata coz nvDsOSD is lazy"
#define PLZPRINTMETA_KLASS "Extracter/Metadata"
#define PLZPRINTMETA_DESCRIPTION "Printing out labels and metadata because nvDsOSD doesn't do so :("
#define PLZPRINTMETA_AUTHOR "Salman Maqbool <salmanmaq@gmail.com>"


static void
gst_plzprintmeta_class_init (GstPlzPrintMetaClass * klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;
    GstBaseTransformClass *gstbasetransform_class;

    g_setenv ("DS_NEW_BUFAPI", "1", TRUE);

    gobject_class = (GObjectClass *) klass;
    gstelement_class = (GstElementClass *) klass;
    gstbasetransform_class = (GstBaseTransformClass *) klass;

    gobject_class->set_property = GST_DEBUG_FUNCPTR (gst_plzprintmeta_set_property);
    gobject_class->get_property = GST_DEBUG_FUNCPTR (gst_plzprintmeta_get_property);
    gstbasetransform_class->stop = GST_DEBUG_FUNCPTR (gst_plzprintmeta_stop);
    gstbasetransform_class->transform_ip =
        GST_DEBUG_FUNCPTR (gst_plzprintmeta_transform_ip);

    gst_element_class_add_pad_template (gstelement_class,
        gst_static_pad_template_get (&src_template));
    gst_element_class_add_pad_template (gstelement_class,
        gst_static_pad_template_get (&sink_template));

    gst_element_class_set_static_metadata (gstelement_class,
        PLZPRINTMETA_LONG_NAME, PLZPRINTMETA_KLASS,
        PLZPRINTMETA_DESCRIPTION, PLZPRINTMETA_AUTHOR);
}


static void
gst_plzprintmeta_init (GstPlzPrintMeta * plzprintmeta)
{
    GstBaseTransform *btrans = GST_BASE_TRANSFORM (plzprintmeta);
    gst_base_transform_set_in_place (GST_BASE_TRANSFORM (btrans), TRUE);
    gst_base_transform_set_passthrough (GST_BASE_TRANSFORM (btrans), TRUE);
    if (!_dsmeta_quark)
    {
        _dsmeta_quark = g_quark_from_static_string (NVDS_META_STRING);
    }
}


static void
gst_plzprintmeta_set_property (GObject * object, guint prop_id,
                            const GValue * value, GParamSpec * pspec)
{
    GstPlzPrintMeta * filter = GST_PLZPRINTMETA (object);

    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}


static void
gst_plzprintmeta_get_property (GObject * object, guint prop_id,
                            GValue * value, GParamSpec * pspec)
{
    GstPlzPrintMeta * filter = GST_PLZPRINTMETA (object);

    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}


static gboolean
gst_plzprintmeta_stop (GstBaseTransform * btrans)
{
    GstPlzPrintMeta * plzprintmeta = GST_PLZPRINTMETA (btrans);
    return TRUE;
}


static GstFlowReturn
gst_plzprintmeta_transform_ip (GstBaseTransform * btrans, GstBuffer * buffer)
{
    GstPlzPrintMeta * plzprintmeta = GST_PLZPRINTMETA (btrans);
    static guint use_device_mem = 0;

    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (buffer);

    /* Iterate each frame metadata in batch */
    for (NvDsMetaList * l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next)
    {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) l_frame->data;
        guint frame_number = frame_meta->frame_num;

        /* Iterate object metadata in frame */
        for (NvDsMetaList * l_obj = frame_meta->obj_meta_list; l_obj != NULL; l_obj = l_obj->next)
        {
            NvDsObjectMeta *obj_meta = (NvDsObjectMeta *) l_obj->data;
            guint primary_class_id = obj_meta->class_id;
            gchar* primary_label = obj_meta->obj_label;

            g_print(
                "Frame Number: %i, Primary Class ID: %i, Primary Label: %s",
                frame_number, primary_class_id, primary_label
            );

            /* Iterate user metadata in object to search SGIE's tensor data */
            for (NvDsMetaList * l_user = obj_meta->obj_user_meta_list; l_user != NULL; l_user = l_user->next)
            {
                NvDsUserMeta *user_meta = (NvDsUserMeta *) l_user->data;
                if (user_meta->base_meta.meta_type != NVDSINFER_TENSOR_OUTPUT_META)
                    continue;

                /* convert to tensor metadata */
                NvDsInferTensorMeta *meta = (NvDsInferTensorMeta *) user_meta->user_meta_data;

                for (unsigned int i = 0; i < meta->num_output_layers; i++)
                {
                    NvDsInferLayerInfo *info = &meta->output_layers_info[i];
                    info->buffer = meta->out_buf_ptrs_host[i];
                    if (use_device_mem && meta->out_buf_ptrs_dev[i])
                    {
                        cudaMemcpy(meta->out_buf_ptrs_host[i], meta->out_buf_ptrs_dev[i],
                          info->inferDims.numElements * 4, cudaMemcpyDeviceToHost);
                    }
                }

                NvDsInferDimsCHW dims;

                getDimsCHWFromDims (dims, meta->output_layers_info[0].inferDims);
                unsigned int numClasses = dims.c;
                float *outputCoverageBuffer = (float *) meta->output_layers_info[0].buffer;
                float maxProbability = 0;
                bool attrFound = false;
                NvDsInferAttribute attr;

                /* Iterate through all the probabilities that the object belongs to
                 * each class. Find the maximum probability and the corresponding class
                 * which meets the minimum threshold. */
                for (unsigned int c = 0; c < numClasses; c++)
                {
                    float probability = outputCoverageBuffer[c];
                    if (probability > 0.3 && probability > maxProbability)
                    {
                        maxProbability = probability;
                        attrFound = true;
                        attr.attributeIndex = 0;
                        attr.attributeValue = c;
                        attr.attributeConfidence = probability;
                    }
                    // g_print(", Class index: %i, Probability: %f", c, probability);
                }

                /* Generate classifer metadata and attach to obj_meta */
                if (attrFound)
                {
                    NvDsClassifierMeta *classifier_meta = nvds_acquire_classifier_meta_from_pool (batch_meta);

                    classifier_meta->unique_component_id = meta->unique_id;

                    NvDsLabelInfo *label_info =
                        nvds_acquire_label_info_meta_from_pool (batch_meta);
                    label_info->result_class_id = attr.attributeValue;
                    label_info->result_prob = attr.attributeConfidence;
                    strcpy (label_info->result_label, sgie_classes_str[label_info->result_class_id]);

                    gchar *temp = obj_meta->text_params.display_text;
                    obj_meta->text_params.display_text = g_strconcat (temp, " ", label_info->result_label, nullptr);
                    g_free (temp);

                    nvds_add_label_info_meta_to_classifier (classifier_meta, label_info);
                    nvds_add_classifier_meta_to_object (obj_meta, classifier_meta);
                }
            }

            /* Iterate classifier metadata in frame */
            for (NvDsMetaList *l_cls = obj_meta->classifier_meta_list; l_cls != NULL; l_cls = l_cls->next)
            {
                NvDsClassifierMeta *cls_meta = (NvDsClassifierMeta *) l_cls->data;

                for (NvDsMetaList *l_lbl = cls_meta->label_info_list; l_lbl != NULL; l_lbl = l_lbl->next)
                {
                    NvDsLabelInfo *lbl_info = (NvDsLabelInfo *) (l_lbl->data);
                    guint secondary_class_id = lbl_info->result_class_id;
                    gchar* secondary_label = lbl_info->result_label;
                    gfloat confidence = lbl_info->result_prob;

                    g_print(
                        ", Secondary Class ID: %i, Secondary Label: %s, Confidence: %f",
                        secondary_class_id, secondary_label, confidence
                    );
                }
            }

            g_print("\n");
        }
    }

    use_device_mem = 1 - use_device_mem;
    return GST_FLOW_OK;
}


static gboolean
plzprintmeta_init (GstPlugin * plugin)
{
    GST_DEBUG_CATEGORY_INIT (gst_plzprintmeta_debug, PLZPRINTMETA_NAME,
                             0, PLZPRINTMETA_DESCRIPTION);
    return gst_element_register (plugin, PLZPRINTMETA_NAME, GST_RANK_NONE,
                                 GST_TYPE_PLZPRINTMETA);
}


#ifndef PACKAGE
#define PACKAGE PLZPRINTMETA_NAME
#endif


GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   plzprintmeta,
                   PLZPRINTMETA_NAME,
                   plzprintmeta_init,
                   "gst_version",
                   "LGPL",
                   "GStreamer",
                   "http://gstreamer.net/")
