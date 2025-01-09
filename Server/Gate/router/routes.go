package router

import (
	"Gate/consul"
	"Gate/log"
	"fmt"
	"github.com/gin-gonic/gin"
	"io"
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

	// user服务
	user := r.Group("/user")
	// 获取用户信息
	user.GET("", func(context *gin.Context) {
		forwardRequest(context, "user")
	})
	// 注册
	user.POST("", func(context *gin.Context) {
		forwardRequest(context, "user")
	})
	// 登录
	user.POST("/login", func(context *gin.Context) {
		forwardRequest(context, "user")
	})
	// 获取验证码
	user.GET("/verify_code", func(context *gin.Context) {
		forwardRequest(context, "user")
	})
	// 获取头像
	user.GET("/icon", func(context *gin.Context) {
		forwardRequest(context, "user")
	})
	// 更新头像
	user.POST("/icon", func(context *gin.Context) {
		forwardRequest(context, "user")
	})
	// 修改个人信息
	user.POST("/modify", func(context *gin.Context) {
		forwardRequest(context, "user")
	})

	return r
}

// 辅助函数：复制HTTP头
func copyHeaders(dst, src http.Header) {
	for k, vv := range src {
		for _, v := range vv {
			dst.Add(k, v)
		}
	}
}

// 转发函数
func forwardRequest(c *gin.Context, serviceName string) {
	ip, port, err := consul.GetServiceByName(serviceName)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"code": "500",
			"msg":  "Failed to get service address",
		})
		return
	}
	originalPath := c.Request.URL.Path
	originalQuery := c.Request.URL.RawQuery

	// 构建目标URL，添加api/v1前缀

	targetURL := fmt.Sprintf("http://%s:%d/api/v1", ip, port) + originalPath

	// 如果有查询参数，则附加到目标URL
	if originalQuery != "" {
		targetURL += "?" + originalQuery
	}

	// 创建新的HTTP请求
	req, err := http.NewRequestWithContext(c.Request.Context(), c.Request.Method, targetURL, c.Request.Body)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"code": "500",
			"msg":  "Failed to create request",
		})
		return
	}

	// 复制原始请求头
	copyHeaders(req.Header, c.Request.Header)

	// 发起HTTP请求
	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"code": "500",
			"msg":  "Failed to forward request",
		})
		return
	}
	defer resp.Body.Close()

	// 复制响应头
	copyHeaders(c.Writer.Header(), resp.Header)

	// 设置响应状态码
	c.Writer.WriteHeader(resp.StatusCode)

	// 复制响应体
	_, err = io.Copy(c.Writer, resp.Body)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"code": "500",
			"msg":  "Failed to copy response body",
		})
		return
	}
}
