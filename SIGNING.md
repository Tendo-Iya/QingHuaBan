# 发行者签名（HAVEGET工作室）

> **当前状态（自签名已交付）**：`轻画板.exe` 已使用自签名证书签名，发布者为 `CN=HAVEGET工作室, O=HAVEGET工作室`，含 DigiCert RFC3161 时间戳；在安装信任证书的当前用户下签名为 `Valid`。  
> 自签名不是公共 CA 证书。微软官方说明：自签名在 SmartScreen 中与未签名等效；直接运行带下载标记的单个 EXE 仍会弹 SmartScreen。完整包用 `install-and-run.cmd` 安装信任证书、解除下载标记后，本机 Win11 实测应用本体不再弹；首次安装会弹一次 Windows 根证书信任确认。  
> SmartScreen 的“发布者”来自 **Authenticode 代码签名证书的主题**，不是文件属性里的 CompanyName。  
> 下方内容保留为“公共受信任证书 / Microsoft Store”路线的申请指南，适用于要跨任意电脑稳定零提示的发布。

## 0. 个人/未注册工作室的现实限制

`HAVEGET工作室` 如果只是个人使用的品牌名、没有营业执照或个体工商户登记，**任何正规 CA 都不能把它作为“已验证的组织名称”写进受信任证书**。原因不是技术，而是证书的身份核验规则。

个人开发者可选：

1. **Microsoft Store 个人开发者账号**（不注册公司的首选）  
   由微软签名和分发，安装不受 SmartScreen 拦截；需要 MSIX 打包、开发者账号和商店审核。  
   `HAVEGET工作室` 是否能作为 Publisher display name，需要 Partner Center 的账号/名称验证确认。
2. **注册个体工商户/公司**  
   注册名中带 HAVEGET 字样后，再申请 OV/EV 代码签名证书。证书显示的是注册法定名称，可能是“XX市XX区HAVEGET工作室”，不一定正好是 `HAVEGET工作室`。申请前让 CA 给出最终 Subject 示例。
3. **个人 OV 代码签名证书**  
   可用个人身份申请，但证书主题是你的法定姓名，**不是 HAVEGET工作室**；SmartScreen 可能仍会提示，直到积累信誉。
4. **自签名证书（仅内部）**  
   可以生成 `HAVEGET工作室` 的签名，但只有安装并信任该证书的电脑才会认可；其他电脑/SmartScreen 仍不可靠。适合自己的几台电脑或企业内网，不适合公开发布。

**结论**：不注册主体、不买证书、不通过 Microsoft Store，就无法让公共电脑把 `HAVEGET工作室` 识别为受信任发行者；自签名或改 CompanyName 都不行。

### 未满 18 岁可以走第 3 条吗？

不能一概而论，但**不能独立走完“注册个体工商户 + 购买代码签名证书”**：

- **不满 8 周岁**：无民事行为能力，不能登记为个体工商户。
- **8–18 周岁**：限制民事行为能力；登记机关通常不允许未成年人独立登记个体工商户。  
  例外：**16 周岁以上、以自己的劳动收入为主要生活来源**的未成年人，法律上“视为完全民事行为能力人”（《民法典》第十八条）。部分地方可能据此受理，但各地登记口径不同，CA 通常还会按自己的订阅协议要求成年/监护人。
- **18 周岁以上**：可以独立注册并申请证书。
- **CA 订阅协议**本身是合同，未成年人通常无法独立签署；即使 16–18 岁被视为完全民事行为能力人，CA 也可能要求 18 岁或监护人共同办理。**不要虚报年龄或借用他人身份**，否则会违反 CA 条款，证书可能被吊销。

现实做法：

1. **监护人代为注册个体工商户/公司**，证书和账户都在监护人/注册主体名下；你负责开发。发行者会显示注册主体或监护人姓名，不是“未成年的你”。  
   如果注册名称中包含 HAVEGET，发行者可能显示完整注册名称（如“XX市XX区HAVEGET工作室”），不一定正好是 `HAVEGET工作室`。
2. **Microsoft Store 个人开发者账号**：当前新流程需要政府证件 + 自拍、个人账户免费；官方文档没有写明最低年龄，但未成年人通常需要监护人协助，具体以 Partner Center 验证为准。个人账户是以个人名义发布；要在注册公司名下发布，需要公司账户。
3. **自签名证书**：技术上未成年人和成年人都能生成，但没有公共信任，不能解决其他电脑的 SmartScreen；只适合自己/内网。

参考：
- [晋江市人民政府：限制民事行为能力人能否登记个体工商户](https://www.jinjiang.gov.cn/jdhy/hygq/202511/t20251106_3228351.htm)
- [Microsoft Store 开发者账户类型与流程](https://learn.microsoft.com/zh-cn/windows/apps/publish/partner-center/open-a-developer-account?tabs=individual#1)

## 1. 先确定要哪种发布方式

| 方式 | 发布者显示 | SmartScreen | 需要什么 | 大致周期/费用 |
| --- | --- | --- | --- | --- |
| **EV 代码签名证书** | 核验后的注册主体名称 | 通常立即获得信任 | 企业/个体工商户资料、身份核验、硬件 token 或云 HSM；通常要求成年/注册主体 | 约 3–10 个工作日；约 US$300–700/年 |
| **OV 代码签名证书（组织）** | 核验后的注册主体名称 | 可能仍提示，需积累下载信誉 | 注册主体、资料核验，通常 .pfx 或云签名 | 约 2–7 个工作日；约 US$150–400/年 |
| **OV 代码签名证书（个人）** | 你的法定姓名 | 可能仍提示，需积累信誉 | 个人身份核验；多数 CA 要求成年，部分只向单位签发 | 约 2–7 个工作日；约 US$100–300/年 |
| **Microsoft Store（个人可选）** | 商店账号的 Publisher display name | 由微软签名，安装不受 SmartScreen 拦截 | 开发者账号、MSIX 打包、商店认证 | 注册费 + 审核时间 |
| **Azure Trusted Signing** | 验证后的身份 | 受信任云签名 | Azure 订阅、身份验证；地区可用性需确认 | 按用量计费 |
| **SignPath / Certum 开源签名** | 验证后的项目/组织 | 受信任 | 公开开源项目、申请审核 | 免费或低成本，周期较长 |

**自签名证书不行**：它能生成 `HAVEGET工作室` 的签名，但其他电脑不信任该证书，SmartScreen 仍会拦截，甚至显示“不受信任的发布者”。只适合内部测试。

## 2. 用 HAVEGET工作室 申请时必须注意

1. **证书主题必须包含 HAVEGET工作室**  
   SmartScreen 显示的是证书 Subject（通常为 `O=` 或 `CN=`）。下单时把公司/组织法定名称和需要的显示名称确认清楚。
2. **名称必须能通过 CA 核验**  
   CA 只会把已验证的法定名称写入证书。若 `HAVEGET工作室` 不是营业执照/个体工商户登记名称，CA 可能只能签发个人证书，或要求先注册主体。若实际注册名称不同，最终显示会是注册名称，不是简称。
3. **必须买 Code Signing / Authenticode 证书**  
   不是文档签名、邮件签名或代码托管平台证书。确认 EKU 含 `1.3.6.1.5.5.7.3.3`。
4. **确认支持中文**  
   部分 CA 的证书主题只使用 ASCII/拼音。若必须显示中文 `HAVEGET工作室`，下单前让 CA 确认 Subject 支持中文并给出最终 Subject 示例。
5. **选择受信任 CA 或授权代理**  
   DigiCert、Sectigo、GlobalSign、Entrust、SSL.com、Certum 等。避免来源不明的“代码签名证书”。
6. **准备资料**（以 CA 要求为准）  
   营业执照/个体工商户登记、申请人身份证明、公司地址和电话、域名/邮箱控制权，有时还需要 D-U-N-S 编号。EV 通常还要电话回访。
7. **不要外发私钥**  
   EV 常见为 USB token/云 HSM，私钥不可导出；OV 可能是 `.pfx`。证书和密码只放在本机安全位置。

## 3. 拿到证书后如何签

### 方式 A：证书已安装到当前用户证书存储（EV token 常用）

```powershell
# 查看代码签名证书指纹
Get-ChildItem Cert:\CurrentUser\My -CodeSigningCert | Format-List Subject,Thumbprint,NotAfter

# 签名 + RFC3161 时间戳 + 验证
powershell -ExecutionPolicy Bypass -File .\sign.ps1 -Thumbprint <证书指纹>
```

### 方式 B：CA 给的是 .pfx 文件

```powershell
powershell -ExecutionPolicy Bypass -File .\sign.ps1 -Pfx .\HAVEGET.pfx
# 脚本会安全地提示输入密码，导入当前用户证书存储后签名，并在结束后移除导入的证书
```

### 方式 C：已经安装 signtool，想手动执行

```powershell
signtool sign /sha1 <证书指纹> /fd SHA256 /td SHA256 ^
  /tr http://timestamp.digicert.com .\轻画板.exe

signtool verify /pa /v .\轻画板.exe
```

签名后检查：

```powershell
Get-AuthenticodeSignature .\轻画板.exe |
  Format-List Status,StatusMessage,SignerCertificate
```

期望：
- `Status = Valid`
- `SignerCertificate.Subject` 中包含 `HAVEGET工作室`
- 在干净虚拟机里“下载”该 EXE 再运行，SmartScreen 不再提示

> EV 证书通常立即生效；OV 证书可能需要一段下载信誉积累期。企业 AppLocker/WDAC 策略仍可能单独拦截，这不属于 SmartScreen。

## 4. 时间戳与网络

签名必须带可信时间戳，否则证书到期后签名会失效。脚本默认使用
`http://timestamp.digicert.com`。如果该地址不可达，可换用 CA 提供的时间戳服务；
没有时间戳的签名不建议用于正式发布。

## 5. 当前可执行文件

- 交付文件：`..\轻画板.exe`
- 版本信息公司：`HAVEGET工作室`
- 数字签名：**未签名**
- 签名脚本：`sign.ps1`
- 只有完成第 3 步，SmartScreen 的发行者才会变为证书中的名称。
