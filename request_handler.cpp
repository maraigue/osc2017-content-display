//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"

namespace http {
namespace server {

request_handler::request_handler(const std::string& doc_root)
  : doc_root_(doc_root)
{
}

char hexchar(char num){
  if(num >= 0 && num <= 9){
    return '0' + num;
  }else if(num >= 10 && num <= 15){
    return 'A' - 10 + num;
  }else{
    return 0;
  }
}

void request_handler::append_encoded_name(const std::string & source, std::string & target){
  for(std::size_t i = 0; i < source.length(); ++i){
    switch(source[i]){
    case '-': case '_': case '~': case '.':
      target.append(1, source[i]);
      break;
    default:
      if(source[i] < '0' || (source[i] > '9' && source[i] < 'A') ||
         (source[i] > 'Z' && source[i] < 'a') || source[i] > 'z'){
        target.append(1, '%');
        target.append(1, hexchar((source[i] >> 4) & 0x0F));
        target.append(1, hexchar(source[i] & 0x0F));
      }else{
        target.append(1, source[i]);
      }
      break;
    }
  }
}

void request_handler::append_escaped_name(const std::string & source, std::string & target){
  for(std::size_t i = 0; i < source.length(); ++i){
    switch(source[i]){
    case '&':
      target.append("&amp;");
      break;
    case '>':
      target.append("&gt;");
      break;
    case '<':
      target.append("&lt;");
      break;
    case '"':
      target.append("&quot;");
      break;
    default:
      target.append(1, source[i]);
      break;
    }
  }
}

void request_handler::toppage(reply& rep){
  namespace fs = boost::filesystem;
  
  rep.content.append(
"<html>"
"<head>"
"<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">"
"<title>Sapporo.cpp @ オープンソースカンファレンスHokkaido2017</title>"
"</head>"
"<body>"
"<table style=\"width:100%;height:100%\">"
"<tr><td colspan=\"2\">"
"<h1>Sapporo.cpp @ オープンソースカンファレンスHokkaido2017</h1>"
"<p>このWebサーバはC++ (boost::asio, boost::filesystem) で書かれています。ソースコード→<a href=\"https://github.com/maraigue/osc2017-content-display\" target=\"_blank\">https://github.com/maraigue/osc2017-content-display</a></p>"
"</td></tr>"
"<tr>"
"<td style=\"width:20%;height:100%;vertical-align:top\"><ul>"
  );
  
  fs::path dir(CONTENT_FILE_DIRECTORY);
  fs::directory_iterator end;
  std::size_t counted_files = 0;
  
  for (fs::directory_iterator it(dir); it != end; ++it )
  {
    if( fs::is_regular_file(*it) &&
        it->path().extension().compare(".css") != 0 &&
        it->path().extension().compare(".js") != 0){
      std::string fname_escape, fname_encode;
      append_escaped_name(it->path().filename().native(), fname_escape);
      append_encoded_name(it->path().filename().native(), fname_encode);
      rep.content.append("<li><a href=\"/");
      rep.content.append(fname_encode);
      rep.content.append("\" target=\"content\">");
      rep.content.append(fname_escape);
      rep.content.append("</a>");
      rep.content.append("</li>");
      
      ++counted_files;
    }
  }
  if(counted_files == 0){
    rep.content.append("<li style=\"color:red\">エラー：表示できるファイルが見つかりませんでした。</li>");
  }
  
  rep.content.append(
"</ul></td>"
"<td style=\"width:80%;height:100%\">"
"<iframe style=\"border:none;background:#eeeeee;width:100%;height:100%\" name=\"content\"></iframe>"
"</td>"
"</tr>"
"</table>"
"</body>"
"</html>"
  );
}

void request_handler::handle_request(const request& req, reply& rep)
{
  // Decode url to path.
  std::string request_path;
  if (!url_decode(req.uri, request_path))
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }

  // Request path must be absolute and not contain "..".
  if (request_path.empty() || request_path[0] != '/'
      || request_path.find("..") != std::string::npos)
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }

  // Special handler defined by H.Hiro
  std::string extension;
  
  if (request_path.length() >= 2 && request_path.find_last_of("/\\") == 0){
    // ------------------------------------------------------------
    // Returns a file in CONTENT_FILE_DIRECTORY
    // ------------------------------------------------------------
    // Determine the file extension.
    std::size_t last_dot_pos = request_path.find_last_of(".");
    if (last_dot_pos != std::string::npos)
    {
      extension = request_path.substr(last_dot_pos + 1);
    }

    std::string full_path;
    full_path.reserve(request_path.length() + CONTENT_FILE_DIRECTORY_LENGTH);
    full_path.append(CONTENT_FILE_DIRECTORY);
    full_path.append(request_path);
    std::cerr << full_path << std::endl;

    // Open the file to send back.
    std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
    if (!is)
    {
      rep = reply::stock_reply(reply::not_found);
      return;
    }
    
    // Fill out the reply to be sent to the client.
    char buf[512];
    while (is.read(buf, sizeof(buf)).gcount() > 0)
    {
      rep.content.append(buf, is.gcount());
    }
  }else if(request_path.length() == 1){
    // Returns the index page.
    toppage(rep);
    extension.assign("html");
  }else{
    rep = reply::stock_reply(reply::not_found);
    return;
  }
  
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = mime_types::extension_to_type(extension);
  rep.status = reply::ok;
}

bool request_handler::url_decode(const std::string& in, std::string& out)
{
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i)
  {
    if (in[i] == '%')
    {
      if (i + 3 <= in.size())
      {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value)
        {
          out += static_cast<char>(value);
          i += 2;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
    else if (in[i] == '+')
    {
      out += ' ';
    }
    else
    {
      out += in[i];
    }
  }
  return true;
}

} // namespace server
} // namespace http
