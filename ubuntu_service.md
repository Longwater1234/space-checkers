

```conf
[Unit]
Description=checkers-backend

[Service]
Type=simple
Restart=always
RestartSec=5s
User=ecs-user
Group=ecs-user
Environment="PORT=9876"
ExecStart=/home/ecs-user/goapps/checkers-server/checkers-backend

[Install]
WantedBy=multi-user.target

```