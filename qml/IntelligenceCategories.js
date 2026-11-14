.pragma library
var titles={address:["العنوان","Address"],bank_card:["البطاقة المصرفية","Bank card"],ch_id:["معرّف شركة بريطانية","Companies House ID"],cik:["معرّف SEC","SEC CIK"],company_internal_id:["معرّف الشركة","Company ID"],company_name:["اسم الشركة","Company name"],cryptocurrency:["العملات الرقمية","Cryptocurrency"],discogs_uid:["حساب Discogs","Discogs account"],domain:["النطاق","Domain"],email:["البريد الإلكتروني","Email"],facebook_profile_uid:["حساب Facebook","Facebook account"],geo:["الإحداثيات","Coordinates"],google_profile_uid:["حساب Google","Google account"],image:["الصورة","Image"],imei:["معرّف الجهاز IMEI","Device IMEI"],imo_number:["رقم السفينة IMO","Vessel IMO"],inn:["الرقم الضريبي INN","Tax INN"],instagram_profile_uid:["حساب Instagram","Instagram account"],ip:["عنوان IP","IP address"],linkedin_company_uid:["شركة LinkedIn","LinkedIn company"],linkedin_profile_uid:["حساب LinkedIn","LinkedIn account"],locode:["رمز الموقع LOCODE","Location LOCODE"],mmsi_number:["معرّف السفينة MMSI","Vessel MMSI"],nif:["المعرّف الضريبي NIF","Tax NIF"],odnoklassniki_profile_uid:["حساب OK","OK account"],offshoreleaks_id:["معرّف Offshore Leaks","Offshore Leaks ID"],ogrn:["رقم تسجيل OGRN","OGRN registration"],opencorporates_company_id:["معرّف OpenCorporates","OpenCorporates ID"],opensanctions_object_id:["معرّف OpenSanctions","OpenSanctions ID"],passport:["جواز السفر","Passport"],password:["فحص كلمة مرور","Password check"],person:["اسم شخص","Person"],phone:["رقم الهاتف","Phone number"],query:["بحث نصي","Text query"],registration_number:["رقم التسجيل","Registration number"],ssid:["اسم شبكة Wi-Fi","Wi-Fi SSID"],tail_number:["تسجيل الطائرة","Aircraft registration"],tax_number:["الرقم الضريبي","Tax number"],telegram_profile_uid:["حساب Telegram","Telegram account"],telegram_public_group_screen_name:["مجموعة Telegram","Telegram group"],"telegram_public_group_screen_name+telegram_profile_uid":["عضو مجموعة Telegram","Telegram group member"],twitter_profile_uid:["حساب X","X account"],"uid+service_name":["حساب في خدمة","Service account"],url:["رابط ويب","Web URL"],"us_license_plate+state":["لوحة أمريكية والولاية","US plate and state"],username:["اسم المستخدم","Username"],vessel_name:["اسم السفينة","Vessel name"],vin:["رقم الهيكل VIN","Vehicle VIN"],vk_group_id:["مجموعة VK","VK group"],vk_user_uid:["حساب VK","VK account"],worldwide_plate:["لوحة مركبة","Vehicle plate"],youtube_channel_uid:["قناة YouTube","YouTube channel"],youtube_video_post_id:["فيديو YouTube","YouTube video"],zip_code:["الرمز البريدي","Postal code"]};
function title(name,arabic){return titles[name]?titles[name][arabic?0:1]:String(name).replace(/_/g," ")}
function icon(name){
    if(name==="email")return "mail";
    if(name==="phone")return "phone";
    if(name==="username")return "people";
    if(name==="person"||name.indexOf("profile")>=0||name.indexOf("uid")>=0||name.indexOf("channel")>=0)return "person";
    if(name==="image")return "camera";
    if(name==="bank_card"||name==="cryptocurrency")return "coin_multiple";
    if(name==="passport")return "contact_card";
    if(name==="imei"||name==="ssid")return "phone_laptop";
    if(name==="domain"||name==="ip"||name==="url")return "globe";
    if(name==="address"||name==="geo"||name==="zip_code"||name==="locode")return "flag";
    if(name.indexOf("company")>=0||name==="ch_id")return "building";
    if(name.indexOf("tax")>=0||name==="registration_number"||name==="nif"||name==="ogrn"||name==="inn")return "certificate";
    if(name==="query")return "search_visual";
    if(name==="password")return "document_lock";
    if(name==="vin"||name.indexOf("plate")>=0||name.indexOf("vessel")>=0||name.indexOf("number")>=0)return "toolbox";
    return "database";
}

function fields(specification){
    const fields=[], expression=/(?:^|[,;]\s*)([A-Za-z_][A-Za-z0-9_]*):\s*(string|integer|number|boolean|object|array)/g;
    let match;while((match=expression.exec(String(specification)))!==null)fields.push({name:match[1],type:match[2]});return fields;
}
