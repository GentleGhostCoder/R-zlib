#include <Rcpp.h>
#include <zlib.h>

//' Validate if a File is a Valid Gzip File
//'
//' This function takes a file path as input and checks if it's a valid gzip-compressed file.
//' It reads the file in chunks and tries to decompress it using the zlib library.
//' If any step fails, the function returns \code{FALSE}. Otherwise, it returns \code{TRUE}.
//'
//' @param file_path A string representing the path of the file to validate.
//' @return A boolean value indicating whether the file is a valid gzip file.
//'         \code{TRUE} if the file is valid, \code{FALSE} otherwise.
//' @examples
//' \dontrun{
//' validate_gzip_file("path/to/your/file.gz")
//' }
//' @export
// [[Rcpp::export]]
bool validate_gzip_file(const std::string& file_path) {
  FILE* file = fopen(file_path.c_str(), "rb");
  if (!file) {
    Rcpp::Rcerr << "Failed to open file: " << file_path << std::endl;
    return false;
  }

  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  if (inflateInit2(&strm, 15+32) != Z_OK) {
    fclose(file);
    return false;
  }

  const size_t chunk_size = 1024;
  Bytef in[chunk_size];
  Bytef out[chunk_size];
  int ret;
  bool success = true;  // Flag to indicate success or failure

  do {
    strm.avail_in = fread(in, 1, chunk_size, file);
    if (ferror(file)) {
      Rcpp::Rcerr << "File read error" << std::endl;
      success = false;
      break;
    }
    strm.next_in = in;

    do {
      strm.avail_out = chunk_size;
      strm.next_out = out;
      ret = inflate(&strm, Z_NO_FLUSH);

      if (ret == Z_STREAM_ERROR) {
        Rcpp::Rcerr << "Stream error during decompression" << std::endl;
        success = false;
        break;
      } else if (ret == Z_MEM_ERROR) {
        Rcpp::Rcerr << "Memory error during decompression" << std::endl;
        success = false;
        break;
      } else if (ret == Z_DATA_ERROR) {
        Rcpp::Rcerr << "Data error during decompression" << std::endl;
        success = false;
        break;
      }

    } while (strm.avail_out == 0);

    if (!success) {
      break;
    }

  } while (ret != Z_STREAM_END);

  inflateEnd(&strm);
  fclose(file);

  return success;
}
