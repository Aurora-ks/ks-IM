package ec

const (
	OK = 0
	// 1XXX 业务类
	VerificationNotEqual = 1000
	LoginFailed          = 1001
	// 2XXX 请求类
	BodyParseJson  = 2000
	ParmsInvalid   = 2001
	SendVerifyCode = 2002
	// 3XXX 服务器内
	DBInsert    = 3000
	DBUpdate    = 3001
	DBModify    = 3002
	DBQuery     = 3003
	DBDelete    = 3004
	RedisInsert = 3010
	RedisUpdate = 3011
	RedisModify = 3012
	RedisQuery  = 3013
	RedisDelete = 3014
	JsonMarshal = 3020
	SaveFile    = 3030
)
