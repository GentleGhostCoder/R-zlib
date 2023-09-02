#include <Rcpp.h>
#include <zlib.h>  // Ensure zlib is included

using namespace Rcpp;

//' Retrieve zlib Constants
//'
//' This function returns a list of constants from the zlib C library.
//'
//' The constants are defined as follows:
//' \itemize{
//'   \item \code{DEFLATED}: The compression method, set to 8.
//'   \item \code{DEF_BUF_SIZE}: The default buffer size, set to 16384.
//'   \item \code{DEF_MEM_LEVEL}: Default memory level, set to 8.
//'   \item \code{MAX_WBITS}: Maximum size of the history buffer, set to 15.
//'   \item \code{Z_BEST_COMPRESSION}: Best compression level, set to 9.
//'   \item \code{Z_BEST_SPEED}: Best speed for compression, set to 1.
//'   \item \code{Z_BLOCK}: Block compression mode, set to 5.
//'   \item \code{Z_DEFAULT_COMPRESSION}: Default compression level, set to -1.
//'   \item \code{Z_DEFAULT_STRATEGY}: Default compression strategy, set to 0.
//'   \item \code{Z_FILTERED}: Filtered compression mode, set to 1.
//'   \item \code{Z_FINISH}: Finish compression mode, set to 4.
//'   \item \code{Z_FULL_FLUSH}: Full flush mode, set to 3.
//'   \item \code{Z_HUFFMAN_ONLY}: Huffman-only compression mode, set to 2.
//'   \item \code{Z_NO_COMPRESSION}: No compression, set to 0.
//'   \item \code{Z_NO_FLUSH}: No flush mode, set to 0.
//'   \item \code{Z_PARTIAL_FLUSH}: Partial flush mode, set to 1.
//'   \item \code{Z_RLE}: Run-length encoding compression mode, set to 3.
//'   \item \code{Z_SYNC_FLUSH}: Synchronized flush mode, set to 2.
//'   \item \code{Z_TREES}: Tree block compression mode, set to 6.
//' }
//'
//' @return A named list of zlib constants.
//' @usage constants <- zlib_constants()
//' @keywords internal
// [[Rcpp::export]]
List zlib_constants() {
  // Number of constants
  int num_constants = 21;

  // Instantiate list with num_constants entries
  List result(num_constants);

  // Make a variable to name elements
  CharacterVector namevec(num_constants);

  // Manually set each constant
  result[0] = Z_DEFLATED; namevec[0] = "DEFLATED";
  result[1] = 16384; namevec[1] = "DEF_BUF_SIZE";
  result[2] = 8; namevec[2] = "DEF_MEM_LEVEL";
  result[3] = 15; namevec[3] = "MAX_WBITS";
  result[4] = ZLIB_VERSION; namevec[4] = "ZLIB_RUNTIME_VERSION";
  result[5] = Z_BEST_COMPRESSION; namevec[5] = "Z_BEST_COMPRESSION";
  result[6] = Z_BEST_SPEED; namevec[6] = "Z_BEST_SPEED";
  result[7] = Z_BLOCK; namevec[7] = "Z_BLOCK";
  result[8] = Z_DEFAULT_COMPRESSION; namevec[8] = "Z_DEFAULT_COMPRESSION";
  result[9] = Z_DEFAULT_STRATEGY; namevec[9] = "Z_DEFAULT_STRATEGY";
  result[10] = Z_FILTERED; namevec[10] = "Z_FILTERED";
  result[11] = Z_FINISH; namevec[11] = "Z_FINISH";
  result[12] = Z_FIXED; namevec[12] = "Z_FIXED";
  result[13] = Z_FULL_FLUSH; namevec[13] = "Z_FULL_FLUSH";
  result[14] = Z_HUFFMAN_ONLY; namevec[14] = "Z_HUFFMAN_ONLY";
  result[15] = Z_NO_COMPRESSION; namevec[15] = "Z_NO_COMPRESSION";
  result[16] = Z_NO_FLUSH; namevec[16] = "Z_NO_FLUSH";
  result[17] = Z_PARTIAL_FLUSH; namevec[17] = "Z_PARTIAL_FLUSH";
  result[18] = Z_RLE; namevec[18] = "Z_RLE";
  result[19] = Z_SYNC_FLUSH; namevec[19] = "Z_SYNC_FLUSH";
  result[20] = 6; namevec[20] = "Z_TREES";

  // Add element names
  result.attr("names") = namevec;

  return result;
}
