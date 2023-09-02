#' Evaluate Expression with Public and Private Environments
#'
#' `publicEval` creates an environment hierarchy consisting of
#' public, self, and private environments. The expression `expr` is
#' evaluated within these nested environments, allowing for controlled
#' variable scope and encapsulation.
#'
#' @section Environments:
#' * Public: Variables in this environment are externally accessible.
#' * Self: Inherits from Public and also contains Private and Public as children.
#' * Private: Variables are encapsulated and are not externally accessible.
#'
#' @param expr An expression to evaluate within the constructed environment hierarchy.
#' @param parentEnv The parent environment for the new 'public' environment. Default is the parent frame.
#' @param name Optional name attribute to set for the public environment.
#'
#' @return Returns an invisible reference to the public environment.
#'
#' @usage publicEval(expr, parentEnv = parent.frame(), name = NULL)
#'
#' @keywords internal
#'
#' @examples
#' \dontrun{
#' publicEnv <- publicEval({
#'   private$hidden_var <- "I am hidden"
#'   public_var <- "I am public"
#' }, parentEnv = parent.frame(), name = "MyEnvironment")
#'
#' print(exists("public_var", envir = publicEnv))  # Should return TRUE
#' print(exists("hidden_var", envir = publicEnv))  # Should return FALSE
#' }
#'
#' @rdname publicEval
#' @name publicEval
publicEval <- function(expr, parentEnv = parent.frame(), name=NULL){
  public <- new.env(parent = parentEnv)
  self <- new.env(parent = public)
  private <- new.env(parent = self)
  self$self <- self
  self$public <- public
  self$private <- private

  eval(substitute(expr), envir = self, enclos = .Primitive('baseenv')())

  object_names <- names(self)
  object_names <- object_names[!(object_names %in% c("public","private","self"))]

  if(length(object_names))
    invisible(
      mapply(assign, object_names, mget(object_names, self), list(public),
             SIMPLIFY = FALSE, USE.NAMES = FALSE) )

  if(!is.null(name)&&is.character(name)) attr(public, "name") <- name
  return(invisible(public))
}

# Placeholder for the zlib enrionment
zlib <- NULL

#' .onLoad function for the package
#'
#' This function is automatically called when the package is loaded using
#' `library()` or `require()`. It initializes the package environment,
#' including defining a variety of constants related to the zlib compression
#' library.
#'
#' Specifically, the function assigns a new environment named "zlib" containing
#' constants such as `DEFLATED`, `DEF_BUF_SIZE`, `MAX_WBITS`,
#' and various flush and compression strategies like `Z_FINISH`,
#' `Z_BEST_COMPRESSION`, etc.
#'
#' @seealso [publicEval()] for the method used to set up the public environment.
#' @seealso [zlib_constants()] for the method used to set up the constants in the environment.
#' @name zlib
#' @keywords internal
.onLoad <- function(libname, pkgname) {
  assign("zlib", publicEval({
    # constants
    list2env(zlib_constants(), envir=environment())
    compressobj <- compressobj
    decompressobj <- decompressobj
  }, name="zlib"), envir = getNamespace(pkgname))

}


#' Create a Compression Object
#'
#' `compressobj` initializes a new compression object with specified parameters
#' and methods. The function makes use of `publicEval` to manage scope and encapsulation.
#'
#' @section Methods:
#' * `compress(data)`: Compresses a chunk of data.
#' * `flush()`: Flushes the compression buffer.
#'
#' @param level Compression level, default is -1.
#' @param method Compression method, default is `zlib$DEFLATED`.
#' @param wbits Window bits, default is `zlib$MAX_WBITS`.
#' @param memLevel Memory level, default is `zlib$DEF_MEM_LEVEL`.
#' @param strategy Compression strategy, default is `zlib$Z_DEFAULT_STRATEGY`.
#' @param zdict Optional predefined compression dictionary as a raw vector.
#'
#' @return Returns an environment containing the public methods `compress` and `flush`.
#'
#' @usage compressobj(
#'              level = -1,
#'              method = zlib$DEFLATED,
#'              wbits = zlib$MAX_WBITS,
#'              memLevel = zlib$DEF_MEM_LEVEL,
#'              strategy = zlib$Z_DEFAULT_STRATEGY,
#'              zdict = NULL
#'          )
#'
#' @examples
#' \dontrun{
#' comp_obj <- compressobj(level = 6)
#' compressed_data <- comp_obj$compress("some data")
#' flushed_data <- comp_obj$flush()
#' }
#'
#' @rdname compressobj
#' @name compressobj
#' @export
compressobj <- function(level=-1, method=zlib$DEFLATED, wbits=zlib$MAX_WBITS, memLevel=zlib$DEF_MEM_LEVEL, strategy=zlib$Z_DEFAULT_STRATEGY, zdict=NULL){
  return(publicEval({
    private$pointer <- create_compressor(level = level, method = method, wbits = wbits, memLevel = memLevel, strategy = strategy, zdict=zdict)
    compress <- function(data){
      return(compress_chunk(private$pointer, data))
    }
    flush <- function(mode = zlib$Z_FINISH){
      return(flush_compressor_buffer(private$pointer, mode = mode))
    }
  }))
}


#' Create a new decompressor object
#'
#' Initializes a new decompressor object for zlib-based decompression.
#'
#' @param wbits The window size bits parameter. Default is 0.
#' @return A decompressor object with methods for decompression.
#'
#' @details
#' The returned decompressor object has methods for performing chunk-wise
#' decompression on compressed data using the zlib library.
#'
#' @examples
#' # Create a decompressor with default window size
#' decompressor <- decompressobj()
#'
#' # Create a decompressor with a specific window size
#' decompressor <- decompressobj(wbits = zlib$MAX_WBITS)
#'
#' @export
decompressobj <- function(wbits = 0) {
  return(publicEval({
    private$pointer <- create_decompressor(wbits = wbits)
    decompress <- function(data) {
      return(decompress_chunk(private$pointer, data))
    }
    flush <- function(length = 256L)  {
      return(flush_decompressor_buffer(private$pointer, length = length))
    }
  }))
}
