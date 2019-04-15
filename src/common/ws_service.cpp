#include "ws_service.h"

#include <iostream>
#include <queue>

#include "glog/logging.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "err.h"
#include "converter.h"

namespace babeltrader
{


WsService::WsService(QuoteService *quote_service, TradeService *trade_service)
	: quote_(quote_service)
	, trade_(trade_service)
{
	if (quote_)
	{
		quote_->p_ws_service_ = this;
	}
	if (trade_)
	{
		trade_->p_ws_service_ = this;
	}

	RegisterCallbacks();

	std::thread th(&WsService::MessageLoop, this);
	th.detach();
}

void WsService::onConnection(uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest &req)
{
	LOG(INFO) << "ws connection: " << ws->getAddress().address << ":" << ws->getAddress().port << ", url: " << req.getUrl().toString() << std::endl;
	if (req.getUrl().toString() != "/ws")
	{
		ws->close();
	} 
	else
	{
		std::unique_lock<std::mutex> lock(ws_mtx_);
		ws_set_.insert(ws);
	}
}
void WsService::onDisconnection(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length)
{
	LOG(INFO) << "ws disconnection: " << ws->getAddress().address << ":" << ws->getAddress().port << std::endl;
	{
		std::unique_lock<std::mutex> lock(ws_mtx_);
		ws_set_.erase(ws);
	}
}
void WsService::onMessage(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode)
{
	LOG(INFO).write(message, length);

	rapidjson::Document doc;
	doc.Parse(message, length);

	if (doc.HasParseError()) {
		OnClientMsgError(ws, message, length, 
			BABELTRADER_ERR_WSREQ_FAILED_PARSE, 
			BABELTRADER_ERR_MSG[BABELTRADER_ERR_WSREQ_FAILED_PARSE - BABELTRADER_ERR_BEGIN]);
		return;
	}

	int ret = PutMsg(ws, std::move(doc));
	if (ret != 0) {
		OnClientMsgError(ws, message, length,
			BABELTRADER_ERR_WSREQ_FAILED_TUNNEL,
			BABELTRADER_ERR_MSG[BABELTRADER_ERR_WSREQ_FAILED_TUNNEL - BABELTRADER_ERR_BEGIN]);
	}
}

int WsService::PutMsg(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &&doc)
{
	WsTunnelMsg msg(ws, std::move(doc));

	auto ret = msg_tunnel_.Write(std::move(msg));
	if (ret != muggle::TUNNEL_SUCCESS) {
		return -1;
	}

	return 0;
}

void WsService::SendMsgToClient(uWS::WebSocket<uWS::SERVER> *ws, const char *msg)
{
	std::unique_lock<std::mutex> lock(ws_mtx_);
	if (ws_set_.find(ws) != ws_set_.end()) {
		ws->send(msg);
	}
}

void WsService::MessageLoop()
{
	std::queue<WsTunnelMsg> queue;
	while (true) {
		msg_tunnel_.Read(queue, true);
		while (queue.size()) {
			WsTunnelMsg &msg = queue.front();
			Dispatch(msg.ws_, msg.doc_);
			queue.pop();
		}
	}
}

void WsService::RegisterCallbacks()
{
	if (trade_)
	{
		callbacks_["insert_order"] = std::bind(&TradeService::OnReqInsertOrder, trade_, std::placeholders::_1, std::placeholders::_2);
		callbacks_["cancel_order"] = std::bind(&TradeService::OnReqCancelOrder, trade_, std::placeholders::_1, std::placeholders::_2);
		callbacks_["query_order"] = std::bind(&TradeService::OnReqQueryOrder, trade_, std::placeholders::_1, std::placeholders::_2);
		callbacks_["query_trade"] = std::bind(&TradeService::OnReqQueryTrade, trade_, std::placeholders::_1, std::placeholders::_2);
		callbacks_["query_position"] = std::bind(&TradeService::OnReqQueryPosition, trade_, std::placeholders::_1, std::placeholders::_2);
		callbacks_["query_positiondetail"] = std::bind(&TradeService::OnReqQueryPositionDetail, trade_, std::placeholders::_1, std::placeholders::_2);
		callbacks_["query_tradeaccount"] = std::bind(&TradeService::OnReqQueryTradeAccount, trade_, std::placeholders::_1, std::placeholders::_2);
		callbacks_["query_product"] = std::bind(&TradeService::OnReqQueryProduct, trade_, std::placeholders::_1, std::placeholders::_2);
		callbacks_["query_tradingday"] = std::bind(&TradeService::OnReqQueryTradingDay, trade_, std::placeholders::_1, std::placeholders::_2);
	}
	
}
void WsService::Dispatch(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc)
{
	try
	{
		if (!doc["msg"].IsString()) throw std::runtime_error("field \"msg\" need string");

		auto it = callbacks_.find(doc["msg"].GetString());
		if (it != callbacks_.end())
		{
			it->second(ws, doc);
		}
		else
		{
			OnClientMsgError(ws, doc,
				BABELTRADER_ERR_WSREQ_NOT_HANDLE,
				BABELTRADER_ERR_MSG[BABELTRADER_ERR_WSREQ_NOT_HANDLE - BABELTRADER_ERR_BEGIN]);
		}
	}
	catch (std::exception &e)
	{
		// get error message
		auto error_msg = std::string(
			BABELTRADER_ERR_MSG[BABELTRADER_ERR_WSREQ_FAILED_HANDLE - BABELTRADER_ERR_BEGIN]) + std::string(" - ") + e.what();

		// response error
		OnClientMsgError(ws, doc,
			BABELTRADER_ERR_WSREQ_FAILED_HANDLE,
			error_msg.c_str());
	}
}

void WsService::OnClientMsgError(uWS::WebSocket<uWS::SERVER> *ws, const char *message, size_t length, int error_id, const char  *error_msg)
{
	rapidjson::Document doc;
	doc.Parse<0>(message, length);
	if (doc.HasParseError()) {
		LOG(WARNING) << "failed parse json from message";
		LOG(WARNING).write(message, length);
		return;
	}

	OnClientMsgError(ws, doc, error_id, error_msg);
}
void WsService::OnClientMsgError(uWS::WebSocket<uWS::SERVER> *ws, rapidjson::Document &doc, int error_id, const char  *error_msg)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("msg");
	writer.String("error");
	writer.Key("error_id");
	writer.Int(error_id);
	writer.Key("error_msg");
	writer.String(error_msg);
	writer.Key("data");
	doc.Accept(writer);
	writer.EndObject();

	LOG(INFO) << s.GetString();

	{
		std::unique_lock<std::mutex> lock(ws_mtx_);
		if (ws_set_.find(ws) != ws_set_.end()) {
			ws->send(s.GetString());
		}
	}
}


}