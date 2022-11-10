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

#include "./config.h"
#include <aws/core/Aws.h>
#include <aws/sns/SNSClient.h>
#include <aws/sns/model/PublishRequest.h>
#include <aws/sns/model/PublishResult.h>

#include "base64.h"
#include <nlohmann/json.hpp>
#include "web_server.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <tuple>
#include <bongo.hpp>

#include <urilite.h>
#include <unifex/scheduler_concepts.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/static_thread_pool.hpp>
#include <unifex/then.hpp>
#include <unifex/when_all.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/just.hpp>
#include <unifex/let_error.hpp>
/**
 * Lists topics - demonstrates how to retrieve a list of Amazon SNS topics.
 */
struct AwsSdk {
  Aws::SDKOptions options{};
  AwsSdk() { Aws::InitAPI(options); }
  ~AwsSdk() { Aws::ShutdownAPI(options); }
};
class AwsSns {
  Aws::SNS::SNSClient sns{};

public:
  AwsSns() {}
  bool publish(const std::string& message, const std::string& arn) {
    return publish(
        std::string_view{message.data(), message.length()},
        std::string_view{arn.data(), arn.length()});
  }
  bool publish(std::string_view message, std::string_view arn) const {
    Aws::SNS::Model::PublishRequest psms_req;
    psms_req.SetMessage(message.data());
    psms_req.SetTopicArn(arn.data());

    auto psms_out = sns.Publish(psms_req);
    if (psms_out.IsSuccess()) {
      std::cout << "Message published successfully " << std::endl;
    } else {
      std::cout << "Error while publishing message "
                << psms_out.GetError().GetMessage() << std::endl;
    }
    return psms_out.IsSuccess();
  }
  ~AwsSns() {}
};
using namespace bingo;
using namespace nlohmann;
std::optional<std::tuple<json, json, json, json>>
parse_resouces(const std::string& path) {
  std::string file_name = path + "/ondemand_template.json";
  if (std::filesystem::exists(file_name)) {
    json response = json::parse(std::ifstream(file_name));
    json histstatCutomData =
        json::parse(std::ifstream(path + "/ondemand_custom_data.json"));
    json statitem = json::parse(std::ifstream(path + "/jobinfo_template.json"));
    json histstat = json::parse(std::ifstream(path + "/histstat_item.json"));

    return std::make_optional(
        std::make_tuple(response, histstatCutomData, statitem, histstat));
  }
  return std::nullopt;
}
std::string handle_on_demand_request(auto& req, auto& httpfunc, auto& server) {
  try {
    std::cout << server.root_directory() << "\n";
    auto resources = parse_resouces(server.root_directory());
    if (resources) {
      auto [response, histstatCutomData, statitem, historystatitemCustomdata] =
          resources.value();

      response["eventAttributeValueMap"]["deviceId"]["stringValue"] =
          httpfunc["deviceid"];

      json parmeters = json::parse(req.body());

      auto index = parmeters["ondemandRequest"]["queryParams"]["ordinalIndex"]
                       .get<int>();
      auto limit =
          parmeters["ondemandRequest"]["queryParams"]["limit"].get<int>();
      json historystats;
      while (limit > 0) {
        limit--;
        statitem["jobInfo"]["jobSequenceId"] = index + limit;
        historystats.push_back(statitem);
      }
      histstatCutomData["ondemandResponse"]["historyStats"] = historystats;
      histstatCutomData["originalRequestEncoded"] =
          macaron::Base64::Encode(req.body());
      response["customData"] =
          macaron::Base64::Encode(histstatCutomData.dump());
      AwsSns sns;
      return sns.publish(
                 response.dump(),
                 "arn:aws:sns:us-east-1:695500506655:"
                 "pie-vpc1-crs-pod1-historystats-sns")
          ? "publish success"
          : "publish failed";
    }
    return "Resource Directory Not Set";
  } catch (std::exception& e) {
    std::cout << e.what();
    return e.what();
  }
}
std::string publish_stat_item_request(auto& req, auto& httpfunc, auto& server) {
  try {
    auto resources = parse_resouces(server.root_directory());
    if (resources) {
      auto [response, histstatCutomData, statitem, historystatitemCustomdata] =
          resources.value();

      response["fqResourceName"] =
          "com.hp.cdm.jobManagement.1.resource.historyStatsItem";
      response["eventAttributeValueMap"]["deviceId"]["stringValue"] =
          httpfunc["deviceid"];

      int index = atoi(httpfunc["offset"].data());
      int limit = atoi(httpfunc["count"].data());
      // json parmeters = json::parse(req.body());

      json event = historystatitemCustomdata["events"][0];
      json events;
      while (limit > 0) {
        limit--;
        statitem["jobInfo"]["jobSequenceId"] = index + limit;
        event["eventDetail"]["state"]["reported"] = statitem;
        events.push_back(event);
      }
      historystatitemCustomdata["events"] = events;
      response["customData"] =
          macaron::Base64::Encode(historystatitemCustomdata.dump());
      auto tokenQuery = unifex::just() |
          bongo::process(bongo::verb::post,
                         std::string("https://pie.authz.wpp.api.hp.com/openid/"
                                     "v1/token?grant_type=client_credentials"),
                         bongo::HttpHeader{
                             {"Authorization",
                              "Basic "
                              "eDR3Umt4cFBFWXhUQ1FPQ3JZVXJNOG55NzFWZVcwaDU6U2N2"
                              "TFY1bjV5NWZRWjVTUVhCbjgzZHM5MGlMWDk5Vk4="}},
                         bongo::ContentType{"application/json"}) |
          unifex::then([](auto v) { return json::parse(v.text); }) |
          unifex::then([](json j) {
                          return j["access_token"].get<std::string>();
                        }) |
          bongo::upon_error([](auto v) { return v; });
      auto token = unifex::sync_wait(std::move(tokenQuery)).value();
      auto body = response.dump();
      std::cout << R"(curl POST -h "Authorization:Bearer Token )" << token
                << R"(" )"
                << R"(-h "Content-Type:application/json" )"
                << R"("https://stratus-pie.tropos-rnd.com/v2/)"
                   R"(eventmgtsvc/eventinfos")"
                << R"(--data ")" << "body" << R"(")"
                << "\n";

      auto query =
          unifex::just() |
          bongo::process(
              bongo::verb::post,
              std::string("https://stratus-pie.tropos-rnd.com/v2/"
                          "eventmgtsvc/eventinfos"),
              bongo::HttpHeader{{"Authorization", "Bearer Token " + token}},
              bongo::ContentType{"application/json"},
              bongo::Body{body}) |
          unifex::then([](auto v) { return json::parse(v.text).dump(); }) |
          bongo::upon_error([](auto v) { return v; });
      auto t = unifex::sync_wait(std::move(query)).value();
      return t;
    }
    return "Resource Directory Not Set";
  } catch (std::exception& e) {
    std::cout << e.what();
    return e.what();
  }
}

int main(int argc, char** argv) {
  AwsSdk sdk;

  namespace fs = std::filesystem;
  std::string doc_root = fs::current_path().c_str();
  if (argc > 1) {
    doc_root = argv[1];
  }
  int port = 8088;
  if (argc > 2) {
    port = atoi(argv[2]);
  }
  web_server server;
  auto plain_text_handler = [](auto func) {
    return [func = std::move(func)](auto& req, auto& httpfunc) {
      http::response<http::string_body> resp{http::status::ok, req.version()};
      resp.set(http::field::content_type, "text/plain");
      resp.body() = func(req, httpfunc);
      resp.set(
          http::field::content_length, std::to_string(resp.body().length()));
      return resp;
    };
  };
  server.add_handler(
      {"/health", http::verb::get}, [](auto& req, auto& httpfunc) {
        http::response<http::string_body> resp{http::status::ok, req.version()};
        resp.set(http::field::content_type, "text/plain");
        resp.body() = "I am healthy";
        resp.set(
            http::field::content_length, std::to_string(resp.body().length()));
        return resp;
      });

  server.add_handler(
      {"/liveness", http::verb::get},
      plain_text_handler(
          [](auto& req, auto& httpfunc) { return "Hello I am Alive!!!"; }));
  server.add_handler(
      {"/publish", http::verb::post},
      plain_text_handler([&](auto& req, auto& httpfunc) {
        return publish_stat_item_request(req, httpfunc, server);
      }));
  server.add_handler(
      {"/devices/v1/{deviceid}/ondemand/jobManagement/historyStats/request",
       http::verb::post},
      plain_text_handler([&](auto& req, auto& httpfunc) {
        return handle_on_demand_request(req, httpfunc, server);
      }));

  server.start(doc_root, port);
  return 0;
}
