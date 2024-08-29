#ifndef ZIPP_DATETIME_H
#define ZIPP_DATETIME_H

#include <cstdint>

namespace zipp
{
	struct DateTime
	{
        uint32_t Seconds;
        uint32_t Minutes;
        uint32_t Hour;
        uint32_t Day;
        uint32_t Month;
        uint32_t Year;
	};
}

#endif //!ZIPP_DATETIME_H