#ifndef XTP_TRADE_HANDLER
#define XTP_TRADE_HANDLER

#include <thread>

#include "xtp_trader_api.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common/common_struct.h"
#include "common/ws_service.h"
#include "common/http_service.h"
#include "common/query_cache.h"

#include "conf.h"

using namespace babeltrader;

class XTPTradeHandler : public TradeService, XTP::API::TraderSpi
{
public:
	XTPTradeHandler(XTPTradeConf &conf);

	void run();

public:
	////////////////////////////////////////
	// trade service virtual function
	virtual void InsertOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order) override;
	virtual void CancelOrder(uWS::WebSocket<uWS::SERVER> *ws, Order &order) override;
	virtual void QueryOrder(uWS::WebSocket<uWS::SERVER> *ws, OrderQuery &order_query) override;
	virtual void QueryTrade(uWS::WebSocket<uWS::SERVER> *ws, TradeQuery &trade_query) override;
	virtual void QueryPosition(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &position_query) override;
	virtual void QueryPositionDetail(uWS::WebSocket<uWS::SERVER> *ws, PositionQuery &position_query) override;
	virtual void QueryTradeAccount(uWS::WebSocket<uWS::SERVER> *ws, TradeAccountQuery &tradeaccount_query) override;
	virtual void QueryProduct(uWS::WebSocket<uWS::SERVER> *ws, ProductQuery &query_product) override;

	////////////////////////////////////////
	// spi virtual function
	virtual void OnDisconnected(uint64_t session_id, int reason) override;
	virtual void OnOrderEvent(XTPOrderInfo *order_info, XTPRI *error_info, uint64_t session_id) override;
	virtual void OnTradeEvent(XTPTradeReport *trade_info, uint64_t session_id) override;
	virtual void OnQueryOrder(XTPQueryOrderRsp *order_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) override;
	virtual void OnQueryTrade(XTPQueryTradeRsp *trade_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) override;
	virtual void OnQueryPosition(XTPQueryStkPositionRsp *position, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) override;
	virtual void OnQueryAsset(XTPQueryAssetRsp *asset, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) override;

private:
	void RunAPI();
	void RunService();

	void Reconn();

	////////////////////////////////////////
	// convert common struct to xtp struct
	void ConvertInsertOrderCommon2XTP(Order &order, XTPOrderInsertInfo &req);
	void ConvertQueryOrderCommon2XTP(OrderQuery &order_qry, XTPQueryOrderReq &req);
	void ConvertQueryTradeCommon2XTP(TradeQuery &trade_qry, XTPQueryTraderReq &req);

	////////////////////////////////////////
	// convert xtp struct to common struct
	void ConvertOrderInfoXTP2Common(XTPOrderInfo *order_info, Order &order, OrderStatusNotify &order_status_notify);
	void ConvertTradeReportXTP2Common(XTPTradeReport *trade_info, Order &order, OrderDealNotify &order_deal);
	void ConvertPositionXTP2Common(XTPQueryStkPositionRsp *pPosition, PositionSummaryType2 &position_summary);
	void ConvertTradeAccountXTP2Common(XTPQueryAssetRsp *asset, TradeAccountType2 &trade_account);

	////////////////////////////////////////
	// field convert
	std::string ExtendXTPId(const char *investor_id, const char *trading_day, uint64_t xtp_id);
	uint64_t GetXTPIdFromExtend(const char *ext_ctp_id, int len);

	XTP_MARKET_TYPE ConvertExchangeMarketTypeCommon2XTP(const std::string &exchange);
	XTP_PRICE_TYPE ConvertOrderTypeCommon2XTP(const std::string &product_type, const std::string &order_type);
	XTP_BUSINESS_TYPE ConvertProductTypeCommon2XTP(const std::string &product_type);
	bool ConvertOrderDirCommon2XTP(const std::string &product_type, const std::string &dir, XTP_SIDE_TYPE &xtp_side, XTP_POSITION_EFFECT_TYPE &xtp_position_effect);
	XTP_SIDE_TYPE ConvertOrderSideCommon2XTP_Spot(const char *p_dir, const char *p_action);
	XTP_SIDE_TYPE ConvertOrderSideCommon2XTP_ETF(const char *p_dir, const char *p_action);
	XTP_SIDE_TYPE ConvertOrderSideCommon2XTP_IPO(const char *p_dir, const char *p_action);
	XTP_POSITION_EFFECT_TYPE ConvertOrderPositionEffectCommon2XTP_Spot(const char *p_dir, const char *p_action);

	std::string ConvertMarketTypeExchangeXTP2Common(XTP_MARKET_TYPE market);
	std::string ConvertOrderDirXTP2Common(XTPOrderInfo *order_info);
	std::string ConvertTradeDirXTP2Common(XTPTradeReport *trade_info);
	std::string ConvertPositionDirXTP2Common(XTP_POSITION_DIRECTION_TYPE dir);
	OrderStatusEnum ConvertOrderStatusXTP2Common(XTP_ORDER_STATUS_TYPE order_status);
	OrderSubmitStatusEnum ConvertOrderSubmitStatusXTP2Common(XTP_ORDER_SUBMIT_STATUS_TYPE order_submit_status);
	std::string ConvertAccountTypeXTP2Common(XTP_ACCOUNT_TYPE account_type);

	////////////////////////////////////////
	// order cache
	void RecordOrder(Order &order, uint32_t order_ref, uint64_t session_id);
	bool GetAndCleanRecordOrder(Order *p_order, uint32_t order_ref, int session_id);

	void ThrowXTPLastError(const char *tip_msg);

	////////////////////////////////////////
	// serialize xtp struct to json
	void SerializeXTPOrderInsert(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPOrderInsertInfo &req);
	void SerializeXTPOrderInfo(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPOrderInfo *order_info);
	void SerializeXTPTradeReport(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPTradeReport *trade_info);
	void SerializeXTPQueryOrder(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPQueryOrderReq *qry);
	void SerializeXTPQueryTrade(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPQueryTraderReq *qry);
	void SerializeXTPPosition(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPQueryStkPositionRsp *rsp);
	void SerializeXTPAsset(rapidjson::Writer<rapidjson::StringBuffer> &writer, XTPQueryAssetRsp *rsp);

	////////////////////////////////////////
	// output
	void OutputFrontDisconnected(uint64_t session_id, int reason);
	void OutputOrderInsert(XTPOrderInsertInfo &req);
	void OutputOrderEvent(XTPOrderInfo *order_info, XTPRI *error_info, uint64_t session_id);
	void OutputTradeEvent(XTPTradeReport *trade_info, uint64_t session_id);
	void OutputOrderCancel(uint64_t order_xtp_id, uint64_t session_id);
	void OutputOrderQuery(uint64_t order_xtp_id);
	void OutputOrderQuery(XTPQueryOrderReq *req);
	void OutputTradeQuery(uint64_t trade_xtp_id);
	void OutputTradeQuery(XTPQueryTraderReq *req);
	void OutputPositionQuery(const std::string &symbol);
	void OutputAssetQuery(uint64_t session_id);

	void OutputRspOrderQuery(XTPQueryOrderRsp *order_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);
	void OutputRspTradeQuery(XTPQueryTradeRsp *trade_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);
	void OutputRspPositionQuery(XTPQueryStkPositionRsp *position, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);
	void OutputRspAssetQuery(XTPQueryAssetRsp *asset, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

private:
	XTP::API::TraderApi *api_;
	// NOTE: api_ready_ can't fully guarantee api status ready, but for performance, ignore the problem
	bool api_ready_;
	uint64_t xtp_session_id_;

	XTPTradeConf conf_;

	WsService ws_service_;
	HttpService http_service_;

	std::string trading_day_;
	int req_id_;
	uint32_t order_ref_;

	// order recorder
	std::map<std::string, Order> wait_deal_orders_;
	std::mutex wati_deal_order_mtx_;

	// query cache
	QueryCache qry_cache_;

	// query results
	std::map<int, std::vector<XTPQueryOrderRsp>> rsp_qry_order_caches_;
	std::map<int, std::vector<XTPQueryTradeRsp>> rsp_qry_trade_caches_;
	std::map<int, std::vector<XTPQueryStkPositionRsp>> rsp_qry_position_caches_;
	std::map<int, std::vector<XTPQueryAssetRsp>> rsp_qry_trade_account_caches_;
};


#endif