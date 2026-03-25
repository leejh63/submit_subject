#ifndef EMB_RESULT_H
#define EMB_RESULT_H

typedef enum
{
    EMB_OK = 0,
    EMB_EINVAL = -1,
    EMB_ESTATE = -2,
    EMB_EBUSY = -3,
    EMB_ENOSPACE = -4,
    EMB_ETIMEOUT = -5,
    EMB_EIO = -6,
    EMB_EUNSUPPORTED = -7
} EmbResult;

#endif
