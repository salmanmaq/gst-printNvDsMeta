#ifndef __PLZPRINTMETA_H__
#define __PLZPRINTMETA_H__

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include <gstnvdsmeta.h>

G_BEGIN_DECLS


/* #defines don't like whitespacey bits */
#define GST_TYPE_PLZPRINTMETA \
  (gst_plzprintmeta_get_type())
#define GST_PLZPRINTMETA(obj)                                        \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_PLZPRINTMETA,GstPlzPrintMeta))
#define GST_PLZPRINTMETA_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_PLZPRINTMETA,GstPlzPrintMetaClass))
#define GST_IS_PLZPRINTMETA(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_PLZPRINTMETA))
#define GST_IS_PLZPRINTMETA_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_PLZPRINTMETA))


typedef struct _GstPlzPrintMeta GstPlzPrintMeta;
typedef struct _GstPlzPrintMetaClass GstPlzPrintMetaClass;


struct _GstPlzPrintMeta {
    GstBaseTransform base_trans;
};


struct _GstPlzPrintMetaClass {
    GstBaseTransformClass parent_class;;
};


GType gst_plzprintmeta_get_type (void);


G_END_DECLS

#endif /* __PLZPRINTMETA_H__ */
