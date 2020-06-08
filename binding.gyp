{
	"targets": [
		{
			"target_name": "prlimit",
			"sources": ["src/main.cc"],
			"include_dirs": ["<!(node -e \"require('nan')\")"]
		}
	]
}
