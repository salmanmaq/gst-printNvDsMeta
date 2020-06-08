#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "plzprintmeta.h"

GST_DEBUG_CATEGORY_STATIC (gst_plzprintmeta_debug);
#define GST_CAT_DEFAULT gst_plzprintmeta_debug

static GQuark _dsmeta_quark = 0;


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
#define PLZPRINTMETA_KLASS "Filter/Effect/Video"
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
    NvDsMetaList * l_frame = NULL;
    NvDsMetaList * l_obj = NULL;
    NvDsMetaList * l_cls = NULL;
    NvDsMetaList * l_lbl = NULL;
    NvDsObjectMeta *obj_meta = NULL;
    NvDsClassifierMeta *cls_meta = NULL;
    NvDsLabelInfo *lbl_info = NULL;

    guint frame_number;
    guint primary_class_id;
    gchar* primary_label;
    guint secondary_class_id;
    gchar* secondary_label;
    gfloat confidence;
    guint num_labels;

    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (buffer);

    /* You can modify the code below to print metadata you require.
    For more information, refer to the Nvidia DeepStream SDK API Documentation.*/
    for (l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next) 
    {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) (l_frame->data);
        frame_number = frame_meta->frame_num;

        for (l_obj = frame_meta->obj_meta_list; l_obj != NULL; l_obj = l_obj->next)
        {
            obj_meta = (NvDsObjectMeta *) (l_obj->data);
            primary_class_id = obj_meta->class_id;
            primary_label = obj_meta->obj_label;

            g_print(
                "Frame Number: %i, Primary Class ID: %i, Primary Label: %s\n",
                frame_number, primary_class_id, primary_label
            );
            
            for (l_cls = obj_meta->classifier_meta_list; l_cls != NULL; l_cls->next)
            {
                cls_meta = (NvDsClassifierMeta *) (l_cls->data);
                num_labels = cls_meta->num_labels;

                for (l_lbl = cls_meta->label_info_list; l_lbl != NULL; l_lbl->next)
                {
                    lbl_info = (NvDsLabelInfo *) (l_lbl->data);
                    secondary_class_id = lbl_info->result_class_id;
                    secondary_label = lbl_info->result_label;
                    confidence = lbl_info->result_prob;

                    g_print(
                        ", Secondary Class ID: %i, Secondary Label: %s, Confidence: %f\n",
                        secondary_class_id, secondary_label, confidence
                    );
                }
            }
        }       
    }

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
