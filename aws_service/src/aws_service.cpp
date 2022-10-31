/*
   Copyright 2010-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
   This file is licensed under the Apache License, Version 2.0 (the "License").
   You may not use this file except in compliance with the License. A copy of
   the License is located at
    http://aws.amazon.com/apache2.0/
   This file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied. See the License for the
   specific language governing permissions and limitations under the License.
*/

#include <aws/core/Aws.h>
#include <aws/sns/SNSClient.h>
#include <aws/sns/model/PublishRequest.h>
#include <aws/sns/model/PublishResult.h>

#include "web_server.hpp"
#include <filesystem>
#include <iostream>

/**
 * Lists topics - demonstrates how to retrieve a list of Amazon SNS topics.
 */
struct AwsSdk{
  Aws::SDKOptions options{};
  AwsSdk(){
    Aws::InitAPI(options);
  }
  ~ AwsSdk(){
    Aws::ShutdownAPI(options);
  }
};
class AwsSns
{
  AwsSdk sdk;
  Aws::SNS::SNSClient sns{};
  public:
  AwsSns(){
      
  }
  bool publish(const std::string& message, const std::string& arn){
    return publish(std::string_view{message.data(),message.length()},
            std::string_view{arn.data(),arn.length()});
  }
  bool publish(std::string_view message, std::string_view arn) const{
   
    Aws::SNS::Model::PublishRequest psms_req;
    psms_req.SetMessage(message.data());
    psms_req.SetTopicArn(arn.data());

    auto psms_out = sns.Publish(psms_req);
    if (psms_out.IsSuccess())
    {
      std::cout << "Message published successfully " << std::endl;
    }
    else
    {
      std::cout << "Error while publishing message " << psms_out.GetError().GetMessage()
        << std::endl;
    }
    return psms_out.IsSuccess();
  }
  ~AwsSns(){
    
  }
};
int main(int argc, char ** argv)
{
  using namespace bingo;
  AwsSns sns;
  namespace fs = std::filesystem;
  std::string doc_root = fs::current_path().c_str();
  if (argc > 1) {
    doc_root = argv[1];
  }
  web_server server;
  auto plain_text_handler= [](auto func){
    return [func=std::move(func)](auto& req,auto& httpfunc){
      http::response<http::string_body> resp{http::status::ok, req.version()};
      resp.set(http::field::content_type, "text/plain");
      resp.body()=func(req,httpfunc);
      resp.set(http::field::content_length, std::to_string(resp.body().length()));
      return resp;
    };
  };
  server.add_handler({"/health",http::verb::get},[](auto& req,auto& httpfunc){
    http::response<http::string_body> resp{http::status::ok, req.version()};
    resp.set(http::field::content_type, "text/plain");
    resp.body()="I am healthy";
    resp.set(http::field::content_length, std::to_string(resp.body().length()));
    return resp;

  });

  server.add_handler({"/liveness",http::verb::get},plain_text_handler([](auto& req,auto& httpfunc){
    return "Hello I am Alive!!!";
  }));
  server.add_handler({"/publish",http::verb::post},plain_text_handler([&](auto& req,auto& httpfunc){
    
    return sns.publish(req.body(),httpfunc["arn"]) ? "publish success":"publish failed";
  }));
  server.add_handler({"/devices/v1/{deviceid}/ondemand/jobManagement/historyStats/request",http::verb::post},plain_text_handler([&](auto& req,auto& httpfunc){
    
    return sns.publish(req.body(),httpfunc["arn"]) ? "publish success":"publish failed";
  }));
  
  server.start(doc_root,8088);
  return 0;
}
