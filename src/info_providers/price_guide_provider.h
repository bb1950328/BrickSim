

#ifndef BRICKSIM_PRICE_GUIDE_PROVIDER_H
#define BRICKSIM_PRICE_GUIDE_PROVIDER_H

//price guide 3001.dat CHF black: https://www.bricklink.com/v2/catalog/catalogitem_pgtab.page?idItem=264&idColor=11&st=2&gm=0&gc=0&ei=0&prec=4&showflag=0&showbulk=0&currency=136
//gc: group by currency

//allvars: https://www.bricklink.com/js/allVars.js

//https://www.bricklink.com/v2/catalog/catalogitem.page?P=3001    has this in html:
//var		_var_item		=	{
//	idItem:			264
//,	type:			'P'
//,	typeName:		'Part'
//,	itemno:			'3001'
//,	itemnoBase:		'3001'
//,	itemStatus:		'A'
//,	invStatus:		'X'
//,	itemSeq:		'0'
//,	idColorDefault:	7
//,	typeImgDefault:	'J'
//,	catID:			'5'
//,	idColorForPG:	-1
//,	strMainSImgUrl:	'//img.bricklink.com/ItemImage/PT/7/3001.t1.png'
//,	strMainLImgUrl:	'//img.bricklink.com/ItemImage/PN/7/3001.png'
//,	strLegacyLargeImgUrl:		'//img.bricklink.com/ItemImage/PL/3001.png'
//,	strLegacyLargeThumbImgUrl:	'//img.bricklink.com/ItemImage/PL/3001.png'
//,	strAssoc1ImgSUrl:			''
//,	strAssoc1ImgLUrl:			''
//,	strAssoc2ImgSUrl:			''
//,	strAssoc2ImgLUrl:			''
//,	strItemName:				'Brick 2 x 4'
//};

namespace price_guide_provider {
    struct PriceGuide {
        bool available;

        std::string currency;

        //these values are undefined if available==false
        int totalLots;
        int totalQty;
        float minPrice;
        float avgPrice;
        float qtyAvgPrice;
        float maxPrice;
    };
    bool initialize();

    PriceGuide getPriceGuide(const std::string& partCode, const std::string& currencyCode, const std::string& colorName, bool forceRefresh = false);
    std::optional<PriceGuide> getPriceGuideIfCached(const std::string& partCode, const std::string& currencyCode, const std::string& colorName);
}

#endif //BRICKSIM_PRICE_GUIDE_PROVIDER_H
