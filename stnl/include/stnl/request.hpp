#ifndef STNL_REQUEST_HPP
#define STNL_REQUEST_HPP

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/json.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>


namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
namespace fs = boost::filesystem;

namespace STNL {

  struct UploadedFile {
    std::string name;
    std::string filename;
    fs::path file;  // Path to temporary uploaded file
  };

  class Request {
    public:
      static Request parse(const http::request<http::string_body>& httpReq);
      http::request<http::string_body> GetHttpReq() const;
      std::vector<UploadedFile> files() const;
      boost::json::object data() const;
      boost::json::object query() const;
    
    private:
      Request(const http::request<http::string_body>& req);
      http::request<http::string_body> httpReq_;
      boost::json::object data_;
      boost::json::object query_;
      std::vector<UploadedFile> parsed_files_;
      void parse_request_();
      void parse_query_params_();
      void parse_json_body_();
      void parse_multipart_();
      void parse_urlencoded_body_();
      std::string get_temp_upload_dir_();
  };

} // namespace STNL

#endif // STNL_REQUEST_HPP