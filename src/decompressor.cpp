#include <Rcpp.h>
#include <zlib.h>

using namespace Rcpp;

struct Decompressor {
  z_stream strm{};
  std::vector<uint8_t> buffer;
};

//' Create a new decompressor object
//'
//' Initialize a new decompressor object for zlib-based decompression.
//' @param wbits The window size bits parameter. Default is 0.
//' @return A SEXP pointer to the new decompressor object.
//' @examples
//' decompressor <- create_decompressor()
//' @export
// [[Rcpp::export]]
SEXP create_decompressor(int wbits = 0) {
  Decompressor* decompressor = nullptr;  // Initialize to nullptr

  try {
    decompressor = new Decompressor();

    decompressor->strm.zalloc = Z_NULL;
    decompressor->strm.zfree = Z_NULL;
    decompressor->strm.opaque = Z_NULL;
    decompressor->strm.avail_in = 0;
    decompressor->strm.next_in = Z_NULL;

    if (inflateInit2(&decompressor->strm, wbits) != Z_OK) {
      throw std::runtime_error("Failed to initialize decompressor");
    }

  } catch (...) {
    delete decompressor;  // Safely delete if an exception is thrown
    throw;  // Re-throw the caught exception
  }

  return XPtr<Decompressor>(decompressor, true);  // Now it's safe to return
}


//' Decompress a chunk of data
//'
//' Perform chunk-wise decompression on a given raw vector using a decompressor object.
//' @param decompressorPtr An external pointer to an initialized decompressor object.
//' @param input_chunk A raw vector containing the compressed data chunk.
//' @return A raw vector containing the decompressed data.
//' @examples
//' rawToChar(decompress_chunk(create_decompressor(), memCompress(charToRaw("Hello, World"))))
//' @export
// [[Rcpp::export]]
RawVector decompress_chunk(SEXP decompressorPtr, const RawVector& input_chunk) {
  XPtr<Decompressor> decompressor(decompressorPtr);
  if (!decompressor) {
    stop("Invalid decompressor object");
  }

  decompressor->buffer.insert(decompressor->buffer.end(), input_chunk.begin(), input_chunk.end());
  decompressor->strm.avail_in = decompressor->buffer.size();
  decompressor->strm.next_in = decompressor->buffer.data();

  size_t available_out = 2 * decompressor->buffer.size();
  std::vector<uint8_t> out(available_out);
  decompressor->strm.avail_out = available_out;
  decompressor->strm.next_out = out.data();

  int ret = inflate(&decompressor->strm, Z_SYNC_FLUSH);
  if (ret == Z_STREAM_END) {
    inflateReset(&decompressor->strm);
  } else if (ret != Z_OK) {
    Rcpp::Rcerr << "zlib error: " << (decompressor->strm.msg ? decompressor->strm.msg : "Unknown error") << std::endl;
    stop("Decompression failed");
  }

  auto diff = static_cast<std::vector<uint8_t>::difference_type>(decompressor->buffer.size() - decompressor->strm.avail_in);
  decompressor->buffer.erase(decompressor->buffer.begin(), decompressor->buffer.begin() + diff);

  auto actual_out_diff = static_cast<std::vector<uint8_t>::difference_type>(available_out - decompressor->strm.avail_out);
  return {out.begin(), out.begin() + actual_out_diff};
}

//' Flush the internal buffer of the decompressor object.
//'
//' This function processes all pending input and returns the remaining uncompressed output.
//' The function uses the provided initial buffer size and dynamically expands it as necessary
//' to ensure all remaining data is decompressed. After calling this function, the
//' decompress_chunk() method cannot be called again on the same object.
//' @param decompressorPtr A SEXP pointer to an existing decompressor object.
//' @param length An optional parameter that sets the initial size of the output buffer. Default is 256.
//' @return A raw vector containing the remaining uncompressed output.
//' @examples
//' decompressor <- create_decompressor()
//' # ... (some decompression actions)
//' flushed_data <- flush_decompressor_buffer(decompressor)
//' @export
// [[Rcpp::export]]
RawVector flush_decompressor_buffer(SEXP decompressorPtr, size_t length = 256) {
    XPtr<Decompressor> decompressor(decompressorPtr);
    if (!decompressor) {
        stop("Invalid decompressor object");
    }

    if (decompressor->buffer.empty()) {
        return RawVector::create();
    }

    decompressor->strm.avail_in = decompressor->buffer.size();
    decompressor->strm.next_in = decompressor->buffer.data();

    std::vector<uint8_t> output(length);
    size_t total_decompressed = 0;

    int ret;
   do {
       if (total_decompressed == output.size()) {
           output.resize(output.size() * 2);  // Double the buffer size
       }

       decompressor->strm.avail_out = output.size() - total_decompressed;
       decompressor->strm.next_out = &output[total_decompressed];

       ret = inflate(&decompressor->strm, Z_NO_FLUSH);

       if (ret < 0 && ret != Z_BUF_ERROR) {
           Rcpp::Rcerr << "zlib error code: " << ret << " - "
                       << (decompressor->strm.msg ? decompressor->strm.msg : "Unknown error") << std::endl;
           stop("Flush failed");
       }

       total_decompressed = output.size() - decompressor->strm.avail_out;

   } while (decompressor->strm.avail_in > 0 || ret == Z_BUF_ERROR);

    output.resize(total_decompressed);  // Trim the output to the actual decompressed size
    decompressor->buffer.clear();       // Clear the internal buffer

    return RawVector(output.begin(), output.end());
}



