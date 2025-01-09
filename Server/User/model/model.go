package model

type User struct {
	Id     int    `json:"id"`
	Name   string `json:"name"`
	Gender int    `json:"gender"`
	Icon   string `json:"icon"`
	Email  string `json:"email"`
	Phone  string `json:"phone"`
}

type Image struct {
	Name     string `json:"name"`
	MimeType string `json:"mimeType"`
	Size     int64  `json:"size"`
	Data     string `json:"data"` // Base64 encoded image data
}
