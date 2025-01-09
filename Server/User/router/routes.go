package router

import (
	"User/log"
	"User/logic"
	"log/slog"

	"github.com/gin-gonic/gin"
)

func SetUp() *gin.Engine {
	r := gin.New()
	r.Use(log.GinLogger(), log.GinRecovery(true))
	r.NoRoute(func(c *gin.Context) {
		c.String(404, "404 not found")
		log.L().Error("Invalid Request", slog.String("url", c.Request.URL.Path))
		return
	})
	// health check
	r.GET("/health", func(context *gin.Context) {
		context.String(200, "OK")
		return
	})

	UserGroup := r.Group("/api/v1/user")
	UserGroup.GET("", logic.GetUserInfo)
	UserGroup.POST("", logic.Register)
	UserGroup.GET("/verify_code", logic.SendVerifyCode)
	UserGroup.POST("/login", logic.Login)
	UserGroup.POST("/modify", logic.ModifyUserInfo)
	UserGroup.GET("/icon", logic.GetIcon)
	UserGroup.POST("/icon", logic.UpdateIcon)
	return r
}
