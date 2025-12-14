// request.cpp - 100% Boost 1.89 + boost::filesystem
#include "stnl/http/request.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>
#include <boost/url.hpp>

#include <boost/json.hpp>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <fstream>
#include <iostream>
#include <map>
#include <vector>

namespace core = boost::core;
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace fs = boost::filesystem;
namespace urls = boost::urls;

namespace algo = boost::algorithm;

using boost::uuids::random_generator;
using boost::uuids::to_string;

namespace STNL {

namespace {
void process_multipart_part(const std::string &body, size_t part_pos, size_t next_boundary, const std::string &headers, const std::string &temp_dir,
                            json::object &data_out, std::vector<UploadedFile> &files_out) {
    std::string name;
    std::string filename;
    auto parse_content_disposition = [&](const std::string &line) {
        if (!algo::istarts_with(line, "Content-Disposition:")) { return; }
        constexpr std::size_t NAME_PREFIX_LEN = 6;      // length of name="
        constexpr std::size_t FILENAME_PREFIX_LEN = 10; // length of filename="
        auto np = line.find("name=\"");
        if (np != std::string::npos) {
            np += NAME_PREFIX_LEN;
            auto end = line.find('"', np);
            if (end != std::string::npos) { name = line.substr(np, end - np); }
        }
        auto fp = line.find("filename=\"");
        if (fp != std::string::npos) {
            fp += FILENAME_PREFIX_LEN;
            auto end = line.find('"', fp);
            if (end != std::string::npos) { filename = line.substr(fp, end - fp); }
        }
    };

    std::vector<std::string> lines;
    algo::split(lines, headers, algo::is_any_of("\r\n"), algo::token_compress_on);
    for (const std::string &line : lines) { parse_content_disposition(line); }

    // Trim trailing CRLF
    size_t content_end = next_boundary;
    while (content_end > part_pos && (body[content_end - 1] == '\r' || body[content_end - 1] == '\n')) { --content_end; }

    std::string content = body.substr(part_pos, content_end - part_pos);

    if (!filename.empty()) {
        // Uploaded file
        std::string uuid_str = to_string(random_generator{}());
        fs::path file_path = fs::path(temp_dir) / uuid_str;

        std::ofstream ofs(file_path.string(), std::ios::binary);
        if (ofs) { ofs.write(content.data(), static_cast<std::streamsize>(content.size())); }
        files_out.emplace_back(UploadedFile{.name = std::move(name), .filename = std::move(filename), .file = std::move(file_path)});
    } else if (!name.empty()) {
        // Regular field
        data_out[name] = content;
    }
}
} // namespace

Request::Request(const http::request<http::string_body> &httpReq) : httpReq_(httpReq) {
    parse_request_();
}

auto Request::parse(const http::request<http::string_body> &httpReq) -> Request {
    return {httpReq};
}

auto Request::GetHttpReq() const -> http::request<http::string_body> {
    return httpReq_;
}

auto Request::headers() const -> std::map<std::string, std::string> {
    return headers_;
}

auto Request::get_temp_upload_dir_() -> std::string {
    fs::path temp_dir = fs::temp_directory_path(); // boost::filesystem
    fs::path upload_dir = temp_dir / "stnl_uploads";

    boost::system::error_code ec;
    fs::create_directories(upload_dir, ec); // non-throwing
    if (ec) {
        // Fallback: try to use a different temp location if needed
        upload_dir = temp_dir / "stnl_uploads";
        fs::create_directories(upload_dir); // or throw if you prefer
    }

    return upload_dir.string();
}

void Request::parse_request_() {
    // headers_[http::to_string(http::field::authorization)] =
    // httpReq_[http::field::authorization].to_string();
    for (const auto &it : httpReq_) {
        std::string key(it.name_string().data(), it.name_string().size());
        std::string val(it.value().data(), it.value().size());
        headers_[std::move(key)] = std::move(val);
    }

    parse_query_params_();

    beast::string_view ct = httpReq_[http::field::content_type];

    if (ct.find("application/json") != beast::string_view::npos) {
        parse_json_body_();
    } else if (ct.find("multipart/form-data") != beast::string_view::npos) {
        parse_multipart_();
    } else if (ct.find("application/x-www-form-urlencoded") != beast::string_view::npos) {
        parse_urlencoded_body_();
    }
}

void Request::parse_json_body_() {
    const std::string &body = httpReq_.body();
    if (body.empty()) { return; }

    boost::system::error_code ec;
    json::value jv = json::parse(body, ec);
    if (ec) {
        data_ = {};
        return;
    }
    if (jv.is_object()) {
        data_ = jv.as_object();
    } else {
        data_ = {}; // or handle array/null if your API allows it
    }
}

void Request::parse_query_params_() {
    boost::system::result<urls::url_view> r = urls::parse_origin_form(httpReq_.target());
    if (r.has_error()) { return; }
    urls::url_view uv = r.value();
    for (const urls::param &param : uv.params()) {
        std::string key = param.key;
        std::string val = param.value;
        query_[key] = val;
    }
}

void Request::parse_urlencoded_body_() {
    const std::string &body = httpReq_.body();
    if (body.empty()) { return; }
    boost::system::result<urls::pct_string_view> r = urls::make_pct_string_view(body);
    if (r.has_error()) { return; }
    urls::pct_string_view sv = r.value();
    std::string body_decoded;
    body_decoded.resize(sv.decoded_size() + 1);
    sv.decode({}, urls::string_token::assign_to(body_decoded));
    body_decoded = "/?" + body_decoded;
    boost::system::result<urls::url_view> r2 = urls::parse_origin_form(body_decoded);
    if (r2.has_error()) { return; }
    urls::url_view uv = r2.value();
    for (const urls::param &param : uv.params()) {
        std::string key = param.key;
        std::string val = param.value;
        data_[key] = val;
    }
}

void Request::parse_multipart_() {
    beast::string_view ct = httpReq_[http::field::content_type];
    auto bpos = ct.find("boundary=");
    if (bpos == beast::string_view::npos) { return; }

    constexpr std::size_t BOUNDARY_PREFIX_LEN = 9; // length of "boundary="
    std::string boundary = "--" + std::string(ct.substr(bpos + BOUNDARY_PREFIX_LEN));
    const std::string &body = httpReq_.body();
    std::string temp_dir = get_temp_upload_dir_();

    size_t pos = 0;

    while ((pos = body.find(boundary, pos)) != std::string::npos) {
        pos += boundary.size();

        // Skip line ending after boundary
        if (body.compare(pos, 2, "\r\n") == 0) {
            pos += 2;
        } else if (pos < body.size() && body[pos] == '\n') {
            ++pos;
        }

        // Closing boundary?
        if (body.compare(pos, 2, "--") == 0) { break; }

        // Find end of headers
        size_t headers_end = body.find("\r\n\r\n", pos);
        if (headers_end == std::string::npos) { break; }

        std::string headers = body.substr(pos, headers_end - pos);
        pos = headers_end + 4; // skip \r\n\r\n

        // Find next boundary
        size_t next_boundary = body.find(boundary, pos);
        if (next_boundary == std::string::npos) { break; }

        process_multipart_part(body, pos, next_boundary, headers, temp_dir, data_, parsed_files_);

        pos = next_boundary;
    }
}

// Public accessors
auto Request::query() const -> json::object {
    return query_;
}
auto Request::data() const -> json::object {
    return data_;
}
auto Request::files() const -> std::vector<UploadedFile> {
    return parsed_files_;
}

} // namespace STNL
