package logic

type Response map[string]any

func OK(data ...any) Response {
	res := make(Response)
	if len(data) == 0 {
		res["data"] = nil
	} else if len(data) == 1 {
		res["data"] = data[0]
	} else {
		res["data"] = data
	}
	res["code"] = "0"
	res["message"] = "OK"
	return res
}

func Res(code int, message string, data ...any) Response {
	var res Response
	res["code"] = code
	res["message"] = message
	if len(data) == 0 {
		res["data"] = nil
	} else if len(data) == 1 {
		res["data"] = data[0]
	} else {
		res["data"] = data
	}
	return res
}
