#include "session.h"

Session::Session()
    :m_playerAcceptTask(new _umap_taskProcess)
    ,m_friendList(new _umap_friendList)
    ,m_viewRole(new _umap_roleSock)
    ,m_viewMonster(new _umap_playerViewMonster)
    ,m_skillTime(new _umap_skillTime)
    ,m_playerGoods(new _umap_roleGoods)
    ,m_playerEqu(new _umap_roleEqu)
//        ,m_playerEquAttr(new _umap_roleEquAttr)
    ,m_playerMoney(new _umap_roleMoney)
    ,m_completeTask(new _umap_completeTask)
    ,m_lootGoods(new _umap_lootGoods)
    ,m_lootPosition(new _umap_lootPosition)
    ,m_interchage(new Interchange)
    ,m_taskGoods( new _umap_taskGoods)
{
    m_publicCoolTime = 0;
    memset(m_goodsPosition, 0, BAGCAPACITY);
    m_skillUseTime = 0;
    m_roleid = 0;
    m_usrid.assign(0);
//        m_nick.assign(0);
}
