#include <Rcpp.h>
#include <zlib.h>

using namespace Rcpp;

struct Compressor {
  z_stream strm{};
  std::vector<uint8_t> buffer;
  RawVector zdict;
};

//' Create a new compressor object
//'
//' Initialize a new compressor object for zlib-based compression with specified settings.
//' @param level Compression level, integer between 0 and 9, or -1 for default.
//' @param method Compression method.
//' @param wbits Window size bits.
//' @param memLevel Memory level for internal compression state.
//' @param strategy Compression strategy.
//' @param zdict Optional predefined compression dictionary as a raw vector.
//' @return A SEXP pointer to the new compressor object.
//' @examples
//' compressor <- create_compressor(level = 6, memLevel = 8)
//' @export
// [[Rcpp::export]]
SEXP create_compressor(int level = -1, int method = 8, int wbits = 15,
                       int memLevel = 8, int strategy = 0, Nullable<RawVector> zdict = R_NilValue) {

  Compressor* compressor = nullptr;  // Initialize to nullptr

  try {
    compressor = new Compressor();

    compressor->strm.zalloc = Z_NULL;
    compressor->strm.zfree = Z_NULL;
    compressor->strm.opaque = Z_NULL;

    // Initialize compressor
    if (deflateInit2(&compressor->strm, level, method, wbits, memLevel, strategy) != Z_OK) {
      throw std::runtime_error("Failed to initialize compressor");
    }

    // Set the compression dictionary if provided
    if (zdict.isNotNull()) {
      RawVector dictVec(zdict);
      compressor->zdict = dictVec;
      if (deflateSetDictionary(&compressor->strm, &dictVec[0], dictVec.size()) != Z_OK) {
        throw std::runtime_error("Failed to set dictionary");
      }
    }

  } catch (...) {
    delete compressor;  // Safely delete if an exception is thrown
    throw;  // Re-throw the caught exception
  }

  return XPtr<Compressor>(compressor, true);  // Now it's safe to return
}


//' @title Compress a Chunk of Data
//'
//' @description Compresses a given chunk of raw binary data using a pre-existing compressor object.
//'
//' @param compressorPtr An external pointer to an existing compressor object.
//' This object is usually initialized by calling a different function like `create_compressor()`.
//'
//' @param input_chunk A raw vector containing the uncompressed data that needs to be compressed.
//'
//' @return A raw vector containing the compressed data.
//'
//' @details
//' This function is primarily designed for use with a compressor object created by `create_compressor()`.
//' It takes a chunk of raw data and compresses it, returning a raw vector of the compressed data.
//'
//' @examples
//' # Create a new compressor object for zlib -> wbts = 15
//' zlib_compressor <- create_compressor(wbits=31)
//' compressed_data <- compress_chunk(zlib_compressor, charToRaw("Hello, World"))
//' compressed_data <- c(compressed_data, flush_compressor_buffer(zlib_compressor))
//' decompressed_data <- memDecompress(compressed_data, type = "gzip")
//' cat(rawToChar(decompressed_data))
//' @export
// [[Rcpp::export]]
RawVector compress_chunk(SEXP compressorPtr, const RawVector& input_chunk) {
  XPtr<Compressor> compressor(compressorPtr);
  if (!compressor) {
    stop("Invalid compressor object");
  }

  compressor->strm.avail_in = input_chunk.size();
  compressor->strm.next_in = (Bytef*)input_chunk.begin();

  std::vector<uint8_t> out(input_chunk.size() * 2);  // Initial estimate
  compressor->strm.avail_out = out.size();
  compressor->strm.next_out = out.data();

  int ret;

  do {
    ret = deflate(&compressor->strm, Z_NO_FLUSH);  // Change to Z_NO_FLUSH

    if (ret < 0) {
      Rcpp::Rcerr << "zlib error: " << zError(ret) << std::endl;  // More detailed error message
      stop("Compression failed");
    }

    if (compressor->strm.avail_out == 0) {
      // Double the output buffer size if needed.
      size_t oldSize = out.size();
      out.resize(oldSize * 2);
      compressor->strm.next_out = out.data() + oldSize;
      compressor->strm.avail_out = oldSize;
    }

  } while (compressor->strm.avail_in > 0);

  auto actual_out_diff = out.size() - compressor->strm.avail_out;
  return {out.begin(), out.begin() + actual_out_diff};
}


//' Flush the internal buffer of the compressor object.
//'
//' This function flushes the internal buffer according to the specified mode.
//' @param compressorPtr A SEXP pointer to an existing compressor object.
//' @param mode A compression flush mode. Default is Z_FINISH.
//'     Available modes are Z_NO_FLUSH, Z_PARTIAL_FLUSH, Z_SYNC_FLUSH, Z_FULL_FLUSH, Z_BLOCK, and Z_FINISH.
//' @return A raw vector containing the flushed output.
//' @examples
//' compressor <- create_compressor()
//' # ... (some compression actions)
//' flushed_data <- flush_compressor_buffer(compressor)
//' @export
// [[Rcpp::export]]
RawVector flush_compressor_buffer(SEXP compressorPtr, int mode = 4) { // Use Z_FINISH here
  XPtr<Compressor> compressor(compressorPtr);
  if (!compressor) {
    stop("Invalid compressor object");
  }

  compressor->strm.avail_in = 0; // No input data since we're just flushing
  compressor->strm.next_in = Z_NULL;

  size_t available_out = 512; // Adjust this value as needed
  std::vector<uint8_t> out(available_out);
  compressor->strm.avail_out = available_out;
  compressor->strm.next_out = out.data();

  int ret = deflate(&compressor->strm, mode);

  if (ret != Z_OK && ret != Z_STREAM_END) {
    Rcpp::Rcerr << "zlib error: " << (compressor->strm.msg ? compressor->strm.msg : "Unknown error") << std::endl;
    stop("Flush failed");
  }

  if (mode == Z_FINISH && ret == Z_STREAM_END) {
    deflateReset(&compressor->strm);
    compressor->buffer.clear(); // Clear the internal buffer as we've flushed it
  }

  auto actual_out_diff = static_cast<std::vector<uint8_t>::difference_type>(available_out - compressor->strm.avail_out);
  return {out.begin(), out.begin() + actual_out_diff};
}
