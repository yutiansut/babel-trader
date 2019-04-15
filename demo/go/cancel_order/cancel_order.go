package main

import (
	"log"
	"time"

	DemoTrade "github.com/MuggleWei/babel-trader/demo/go/demo_trade"
	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
)

func main() {
	log.SetFlags(log.LstdFlags | log.LUTC | log.Lmicroseconds | log.Lshortfile)

	ts := time.Now().Unix()

	// // ctp
	// addr := "127.0.0.1:8001"
	// order := common.MessageOrder{
	// 	UserId:      "weidaizi",
	// 	OutsideId:   "104027_20181119_      163230",
	// 	Market:      common.Market_CTP,
	// 	Exchange:    common.Exchange_SHFE,
	// 	ProductType: common.ProductType_Future,
	// 	Symbol:      "rb",
	// 	Contract:    "1905",
	// 	Timestamp:   ts,
	// }

	// // xtp
	// addr := "127.0.0.1:8002"
	// order := common.MessageOrder{
	// 	UserId:      "weidaizi",
	// 	OutsideId:   "15033731_20181118_36567563152394204",
	// 	Market:      common.Market_XTP,
	// 	Exchange:    common.Exchange_SSE,
	// 	ProductType: common.ProductType_Spot,
	// 	Symbol:      "600519",
	// 	Timestamp:   ts,
	// }

	// // okex spot
	// addr := "127.0.0.1:8005"
	// order := common.MessageOrder{
	// 	UserId:      "weidaizi",
	// 	OutsideId:   "2174354053598208",
	// 	Market:      common.Market_OKEX,
	// 	Exchange:    common.Exchange_OKEX,
	// 	ProductType: common.ProductType_Spot,
	// 	Symbol:      "BTC-USDT",
	// 	Timestamp:   ts,
	// }

	// okex future
	addr := "127.0.0.1:8005"
	order := common.MessageOrder{
		UserId:      "weidaizi",
		OutsideId:   "2174372788575232",
		Market:      common.Market_OKEX,
		Exchange:    common.Exchange_OKEX,
		ProductType: common.ProductType_Future,
		Symbol:      "BTC-USD",
		Contract:    "190329",
		Timestamp:   ts,
	}

	service := DemoTrade.NewDemoTradeService()
	service.Run(addr, func() {
		service.ReqCancelOrder(&order)
	})

}
