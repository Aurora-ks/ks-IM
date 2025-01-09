package router

import (
	"Message/log"
	"Message/logic"
	"github.com/gin-gonic/gin"
	"net/http"
)

func SetUp() *gin.Engine {
	r := gin.New()
	r.Use(log.GinLogger(), log.GinRecovery(true))
	r.NoRoute(func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{
			"code": "404",
		})
	})
	// Health Check
	r.GET("/health", func(context *gin.Context) {
		context.String(http.StatusOK, "")
	})
	// 连接建立
	r.GET("/login/:id", logic.NewConnection)

	return r
}
