/**
 * @file fence_codec.h
 * @brief Wire format and validation of a geofence polygon sent to the collar
 *
 * Pure C, no hardware dependency. The same blob travels over every link
 * (cloud, BLE, serial), so the application code does not depend on the board.
 *
 * Blob layout, all integers little endian:
 *
 *   offset  size  field
 *        0     1  version (1)
 *        1     1  flags (0, reserved)
 *        2     2  number of vertices n
 *        4     4  fence id
 *        8     4  fence version (a newer fence has a larger number)
 *       12     4  first vertex latitude, int32, 1e-7 degrees
 *       16     4  first vertex longitude, int32, 1e-7 degrees
 *       20  4*(n-1)  each next vertex: int16 latitude delta, int16 longitude
 *                    delta, in units of FENCE_CODEC_DELTA_UNIT (1e-6 degrees)
 *     20+4*(n-1)  4  CRC-32 (IEEE 802.3) of every byte before it
 *
 * A delta is computed from the previous decoded vertex, so rounding errors do
 * not add up: each vertex is within 5.5 cm of the requested one. An edge can
 * be at most 3.6 km long in latitude or longitude. The first vertex is not
 * repeated at the end.
 *
 * A fence is accepted only if it is a simple polygon (no edge crosses or
 * touches another), has between 3 and FENCE_CODEC_MAX_VERTICES vertices,
 * lies within 85 degrees of latitude and inside -180..180 degrees of
 * longitude (the geofence engine does not support the antimeridian) and
 * covers at least FENCE_CODEC_MIN_AREA_M2.
 */

#ifndef FENCE_CODEC_H
#define FENCE_CODEC_H

#include <stddef.h>
#include <stdint.h>

#include "geofence.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FENCE_CODEC_VERSION 1

/** Maximum number of vertices; override with -DFENCE_CODEC_MAX_VERTICES=n */
#ifndef FENCE_CODEC_MAX_VERTICES
#define FENCE_CODEC_MAX_VERTICES 128
#endif

#if FENCE_CODEC_MAX_VERTICES > 256
/* The area and crossing tests use int64 sums that are proven for up to 256 vertices. */
#error "FENCE_CODEC_MAX_VERTICES must not exceed 256"
#endif

/** Minimum number of vertices of a polygon */
#define FENCE_CODEC_MIN_VERTICES 3

/** Delta unit, in 1e-7 degrees (1e-6 degrees, about 11 cm) */
#define FENCE_CODEC_DELTA_UNIT 10

/** Smallest accepted area, in square meters (GNSS accuracy is about 2.5 m) */
#define FENCE_CODEC_MIN_AREA_M2 100.0

/** Size of a blob with the smallest valid vertex count */
#define FENCE_CODEC_MIN_SIZE 32

/** Size of a blob with the largest valid vertex count */
#define FENCE_CODEC_MAX_SIZE (20 + 4 * FENCE_CODEC_MAX_VERTICES)

/** Result codes. Zero is success; the rest are negative. */
enum fence_codec_status {
	FENCE_CODEC_OK = 0,
	FENCE_CODEC_ERR_ARG = -1,          /**< NULL argument or bad capacity */
	FENCE_CODEC_ERR_SIZE = -2,         /**< length does not match the header */
	FENCE_CODEC_ERR_CRC = -3,          /**< checksum mismatch */
	FENCE_CODEC_ERR_FORMAT = -4,       /**< unknown version or flags */
	FENCE_CODEC_ERR_VERTEX_COUNT = -5, /**< fewer than 3 or too many vertices */
	FENCE_CODEC_ERR_RANGE = -6,        /**< coordinate out of range */
	FENCE_CODEC_ERR_NOT_SIMPLE = -7,   /**< edges cross, touch or overlap */
	FENCE_CODEC_ERR_AREA = -8,         /**< area below the minimum */
	FENCE_CODEC_ERR_EDGE_TOO_LONG = -9, /**< encode: edge over 3.6 km */
	FENCE_CODEC_ERR_BUFFER = -10       /**< output buffer too small */
};

/** Fence identity carried in the blob header */
typedef struct {
	uint32_t fence_id;      /**< Identifier chosen by the sender */
	uint32_t fence_version; /**< Larger means newer */
	uint16_t num_vertices;  /**< Number of vertices (decode output) */
} fence_codec_header_t;

/**
 * @brief CRC-32 (IEEE 802.3, the zlib one) of a buffer
 */
uint32_t fence_codec_crc32(const uint8_t *data, size_t len);

/**
 * @brief Size in bytes of the blob that holds n vertices
 */
size_t fence_codec_encoded_size(uint16_t n);

/**
 * @brief Encode a polygon into a blob
 *
 * Applies the same checks as fence_codec_decode() to the vertices as they
 * will be decoded, so a blob that encodes always decodes. The header's
 * num_vertices field is ignored: @p n is used.
 *
 * @param header    Fence id and version
 * @param vertices  Polygon vertices, first vertex not repeated
 * @param n         Number of vertices
 * @param buf       Receives the blob
 * @param buf_len   Capacity of @p buf
 * @param out_len   Receives the blob size on success
 * @return FENCE_CODEC_OK or a negative fence_codec_status
 */
int fence_codec_encode(const fence_codec_header_t *header,
		       const geopoint_t *vertices, uint16_t n,
		       uint8_t *buf, size_t buf_len, size_t *out_len);

/**
 * @brief Decode and validate a blob
 *
 * Checks, in this order: length, version and flags, vertex count, exact
 * length, CRC, coordinate ranges, simple polygon, minimum area. Nothing is
 * written to @p out unless every check passes.
 *
 * @param buf      The blob
 * @param len      Its length
 * @param out      Receives the vertices
 * @param out_cap  Capacity of @p out, in vertices
 * @param header   Receives the fence id, version and vertex count
 * @return FENCE_CODEC_OK or a negative fence_codec_status
 */
int fence_codec_decode(const uint8_t *buf, size_t len,
		       geopoint_t *out, size_t out_cap,
		       fence_codec_header_t *header);

#ifdef __cplusplus
}
#endif

#endif /* FENCE_CODEC_H */
