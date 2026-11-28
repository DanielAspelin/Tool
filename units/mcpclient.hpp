class MCPCLIENT : public CLIENT {
public:
    explicit MCPCLIENT(MCPSERVER& s): CLIENT(s,"MCP") {}
};
