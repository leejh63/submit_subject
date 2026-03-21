#ifndef EMB_STATUS_H
#define EMB_STATUS_H

typedef enum
{
    EMB_OK = 0,
    EMB_EINVAL,
    EMB_ESTATE,
    EMB_EBUSY,
    EMB_ENOSPACE,
    EMB_ENOENT,
    EMB_ETIMEOUT,
    EMB_EIO,
    EMB_ECRC,
    EMB_EUNSUPPORTED
} emb_status_t;

#endif
