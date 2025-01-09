package logic

import (
	"User/db/mysql"
	"User/db/redis"
	"User/ec"
	"User/log"
	"User/model"
	"User/settings"
	"database/sql"
	"encoding/base64"
	"encoding/json"
	"errors"
	"fmt"
	"math/rand"
	"net/http"
	"net/smtp"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/gin-gonic/gin"
)

// Register 注册
func Register(c *gin.Context) {
	var req struct {
		Name         string `json:"name"`
		Gender       int    `json:"gender"`
		Password     string `json:"password"`
		Email        string `json:"email"`
		Verification string `json:"verification"`
	}
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "Register Parse Json Error"))
		log.L().Warn("Register Body Parse Json")
		return
	}
	// 判断验证码
	code := redis.GetUserVerifyCode(req.Email)
	if code != req.Verification {
		c.JSON(http.StatusOK, Res(ec.VerificationNotEqual, "验证码错误"))
		return
	}
	// 添加用户数据
	if err := mysql.UserRegister(req.Name, req.Password, req.Email, req.Gender); err != nil {
		c.JSON(http.StatusOK, Res(ec.DBInsert, "DB User Register"))
		log.L().Error("DB User Register", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK())
}

// SendVerifyCode 发送验证码
func SendVerifyCode(c *gin.Context) {
	to := c.Query("email") // user email
	// 生成验证吗
	code := redis.GetUserVerifyCode(to)
	if code == "" {
		// 生成一个介于 10000 到 99999 之间的 5 位随机数
		code = fmt.Sprintf("%d", rand.Intn(90000)+10000)
	}
	if err := redis.SetUserVerifyCode(to, code); err != nil {
		log.L().Warn("Redis Set VerifyCode")
	}
	// 设置SMTP服务
	dsn := fmt.Sprintf("%s:%d", settings.Conf.EmailConfig.Host, settings.Conf.EmailConfig.Port)
	auth := smtp.PlainAuth("", settings.Conf.EmailConfig.From,
		settings.Conf.EmailConfig.Password,
		settings.Conf.EmailConfig.Host)
	// 设置邮件内容
	header := fmt.Sprintf("From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n",
		settings.Conf.EmailConfig.From, to, "验证码")
	body := fmt.Sprintf("验证码为：%s\n3分钟内有效", code)
	message := header + body
	// 发送邮件
	if err := smtp.SendMail(dsn, auth, settings.Conf.EmailConfig.From, []string{to}, []byte(message)); err != nil && !strings.Contains(err.Error(), "short response") {
		c.JSON(http.StatusOK, Res(ec.SendVerifyCode, "SendVerifyCode Error"))
		log.L().Error("SendVerifyCode", log.Error(err))
	} else {
		c.JSON(http.StatusOK, OK())
	}
}

// Login 登录
func Login(c *gin.Context) {
	// 获取参数
	var req struct {
		id       int
		password string
	}
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "Login Parse Json Error"))
		log.L().Warn("Login Body Parse Json")
		return
	}
	// 查询数据库
	user, err := mysql.UserLogin(req.id, req.password)
	if errors.Is(err, sql.ErrNoRows) {
		c.JSON(http.StatusOK, Res(ec.LoginFailed, "Login Failed"))
		return
	} else if err != nil {
		log.L().Error("DB User Login", log.Error(err))
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB User Login"))
		return
	}
	data, err := json.Marshal(user)
	if err != nil {
		log.L().Error("User Data Marshal", log.Error(err))
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "User Data Marshal Failed"))
		return
	}
	c.JSON(http.StatusOK, Res(ec.OK, "OK", data))
}

// ModifyUserInfo 修改信息
func ModifyUserInfo(c *gin.Context) {
	// TODO:加入邮箱和密码验证
	var req *model.User
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "User Modify Info Parse Json"))
		log.L().Error("User Modify Parse Json", log.Error(err))
		return
	}
	// 写入数据库
	if err := mysql.UserModify(req); err != nil {
		c.JSON(http.StatusOK, Res(ec.DBModify, "User Modify Info Failed"))
		log.L().Error("User Modify Info Failed", log.Error(err))
	}
}

// GetIcon 获取头像
func GetIcon(c *gin.Context) {
	var req struct {
		Id []int `json:"id"`
	}
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "Get Icon Parse Json"))
		log.L().Error("Get Icon Parse Json", log.Error(err))
		return
	}
	// 读取文件 TODO:mysql读取
	iconPath, err := mysql.GetIcon(req.Id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "Query Icon"))
		log.L().Error("Get Icon", log.Error(err))
		return
	}
	images := make([]*model.Image, len(req.Id))
	imagesDir := settings.Conf.ServerConfig.FilePath + "/icon"
	for i, path := range iconPath { // fs://id.xxx
		filePath := filepath.Join(imagesDir, path[5:])
		// 读取文件
		data, err := os.ReadFile(filePath)
		if err != nil {
			log.L().Error(fmt.Sprintf("Load User Icon %s", filePath), log.Error(err))
			continue
		}
		// 读取文件信息
		fileInfo, err := os.Stat(filePath)
		if err != nil {
			log.L().Error(fmt.Sprintf("Load User Icon Info %s", filePath), log.Error(err))
			continue
		}

		image := &model.Image{
			Name:     fileInfo.Name(),
			MimeType: filepath.Ext(filePath),
			Size:     fileInfo.Size(),
			Data:     base64.StdEncoding.EncodeToString(data),
		}
		images[i] = image
	}
	// 发送数据
	jsonData, err := json.Marshal(images)
	if err != nil {
		log.L().Error("Send User Icon", log.Error(err))
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "User Icon Parse Json"))
		return
	}
	fmt.Println(string(jsonData))
	c.JSON(http.StatusOK, OK(jsonData))
}

// UpdateIcon 更新头像
func UpdateIcon(c *gin.Context) {
	// TODO:加入hash校验，避免重复写入
	var req struct {
		Id          int `json:"id"`
		model.Image `json:"image"`
	}
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "Update Icon Parse Json"))
		log.L().Error("Update Icon Parse Json", log.Error(err))
		return
	}
	// 写入数据库
	if err := mysql.UpdateIcon(req.Id, &req.Image); err != nil {
		c.JSON(http.StatusOK, Res(ec.DBUpdate, "Update Icon"))
		log.L().Error("Update Icon", log.Error(err))
		return
	}
	// 写入文件
	data, err := base64.StdEncoding.DecodeString(req.Image.Data)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.SaveFile, "Icon Base64 Parse Failed"))
		log.L().Error("Icon Base64 Parse", log.Error(err))
		return
	}
	filePath := fmt.Sprintf("%s/icon/%d%s", settings.Conf.ServerConfig.FilePath, req.Id, req.Image.MimeType)
	if err = os.WriteFile(filePath, data, 0644); err != nil {
		c.JSON(http.StatusOK, Res(ec.SaveFile, "Save Icon"))
		log.L().Error("Save Icon", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK())
	return
}

// GetUserInfo 获取用户信息
func GetUserInfo(c *gin.Context) {
	id := c.Query("id")
	uid, err := strconv.Atoi(id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "User Id Parse Failed"))
		log.L().Error("User Id Parse Failed", log.Error(err))
		return
	}
	user, err := mysql.GetUser(uid)
	if errors.Is(err, sql.ErrNoRows) {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "User Not Exist"))
		return
	} else if err != nil {
		log.L().Error("DB Get User", log.Error(err))
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get User"))
		return
	}
	data, err := json.Marshal(user)
	if err != nil {
		log.L().Error("User Data Marshal", log.Error(err))
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "User Data Marshal Failed"))
		return
	}
	c.JSON(http.StatusOK, OK(data))
	return
}
