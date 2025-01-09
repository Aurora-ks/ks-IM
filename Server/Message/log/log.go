package log

import (
	"github.com/gin-gonic/gin"
	"gopkg.in/natefinch/lumberjack.v2"
	"log/slog"
	"net"
	"net/http"
	"net/http/httputil"
	"os"
	"runtime/debug"
	"strings"
	"time"
)

var log *slog.Logger

func Init() {
	lumberjackLogger := &lumberjack.Logger{
		Filename:   "./server.log",
		MaxSize:    10,
		MaxBackups: 2,
		MaxAge:     28,
		Compress:   true,
	}
	log = slog.New(slog.NewJSONHandler(lumberjackLogger, &slog.HandlerOptions{Level: slog.LevelInfo}))
}

func L() *slog.Logger {
	return log
}

func Error(err error) slog.Attr {
	return slog.Any("error", err)
}
func String(k, v string) slog.Attr          { return slog.String(k, v) }
func Int(k string, v int) slog.Attr         { return slog.Int(k, v) }
func Int64(k string, v int64) slog.Attr     { return slog.Int64(k, v) }
func Uint64(k string, v uint64) slog.Attr   { return slog.Uint64(k, v) }
func Float64(k string, v float64) slog.Attr { return slog.Float64(k, v) }
func Bool(k string, v bool) slog.Attr       { return slog.Bool(k, v) }
func Any(k string, v any) slog.Attr         { return slog.Any(k, v) }
func Time(k string, v time.Time) slog.Attr  { return slog.Time(k, v) }

func GinLogger() gin.HandlerFunc {
	return func(c *gin.Context) {
		start := time.Now()
		path := c.Request.URL.Path
		query := c.Request.URL.RawQuery
		c.Next()

		cost := time.Since(start)
		log.Info(path,
			slog.Int("status", c.Writer.Status()),
			slog.String("method", c.Request.Method),
			slog.String("path", path),
			slog.String("query", query),
			slog.String("ip", c.ClientIP()),
			slog.String("user-agent", c.Request.UserAgent()),
			slog.String("errors", c.Errors.ByType(gin.ErrorTypePrivate).String()),
			slog.Duration("cost", cost),
		)
	}
}

func GinRecovery(stack bool) gin.HandlerFunc {
	return func(c *gin.Context) {
		defer func() {
			if err := recover(); err != nil {
				// Check for a broken connection, as it is not really a
				// condition that warrants a panic stack trace.
				var brokenPipe bool
				if ne, ok := err.(*net.OpError); ok {
					if se, ok := ne.Err.(*os.SyscallError); ok {
						if strings.Contains(strings.ToLower(se.Error()), "broken pipe") || strings.Contains(strings.ToLower(se.Error()), "connection reset by peer") {
							brokenPipe = true
						}
					}
				}

				httpRequest, _ := httputil.DumpRequest(c.Request, false)
				if brokenPipe {
					log.Error(c.Request.URL.Path,
						slog.Any("error", err),
						slog.String("request", string(httpRequest)),
					)
					// If the connection is dead, we can't write a status to it.
					c.Error(err.(error)) // nolint: errcheck
					c.Abort()
					return
				}

				if stack {
					log.Error("[Recovery from panic]",
						slog.Any("error", err),
						slog.String("request", string(httpRequest)),
						slog.String("stack", string(debug.Stack())),
					)
				} else {
					log.Error("[Recovery from panic]",
						slog.Any("error", err),
						slog.String("request", string(httpRequest)),
					)
				}
				c.AbortWithStatus(http.StatusInternalServerError)
			}
		}()
		c.Next()
	}
}
