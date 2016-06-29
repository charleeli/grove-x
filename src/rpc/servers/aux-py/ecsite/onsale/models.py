# coding=utf-8
from django.db import models


# 商品标签
class WareLabel(models.Model):
    label_type_choices = [(0, '单商品标签'), (1, '多商品标签')]
    scope_type_choices = [(0, '全平台'), (1, '店铺内')]
    sub_type_choices = [(0, '商品白名单'), (1, '商品黑名单'), (2, '全部商品'), ]

    name = models.CharField("商品标签名称", max_length=100)
    label_type = models.PositiveSmallIntegerField('商品标签类型', default=0, choices=label_type_choices)
    scope_type = models.PositiveSmallIntegerField("使用范围", default=0, choices=scope_type_choices)
    sub_type = models.PositiveSmallIntegerField("商品名单子类型", default=0, choices=sub_type_choices)
    seller_id = models.BigIntegerField("卖家ID")
    seller_slug = models.CharField("卖家slug", max_length=20)
    create_time = models.DateTimeField('创建时间', auto_now_add=True)
    update_time = models.DateTimeField('更新时间', auto_now=True)
    create_man = models.CharField("创建人", max_length=20)

    def __unicode__(self):
        return self.name


# 商品标签-商品表
class WareLabelWares(models.Model):
    ware_label_id = models.PositiveIntegerField("商品标签ID", db_index=True)
    ware_id = models.BigIntegerField("商品ID")
    create_time = models.DateTimeField('创建时间', auto_now_add=True)

    class Meta:
        unique_together = (("ware_label_id", "ware_id"),)

    def __unicode__(self):
        return str(self.ware_label_id) + "_" + str(self.ware_id)


# 赠品表
class Present(models.Model):
    step_id = models.PositiveIntegerField("阶梯ID", db_index=True)
    sku_id = models.BigIntegerField("商品ID")
    sku_price = models.DecimalField('商品价格', max_digits=11, decimal_places=2, default=0)
    sku_count = models.PositiveIntegerField("商品个数", db_index=True)
    sku_slug = models.CharField("商品slug", max_length=20)

    class Meta:
        unique_together = (("step_id", "sku_id"), )

    def __unicode__(self):
        return self.sku_slug


# 优惠阶梯表
class Step(models.Model):
    onsale_group_id = models.PositiveIntegerField("促销活动ID")
    full_credit = models.DecimalField('购满金额', max_digits=11, decimal_places=2, default=0)
    favor_credit = models.DecimalField('免减金额', max_digits=11, decimal_places=2, default=0)
    favor_rate = models.FloatField('打折折扣率', default=0)

    full_count = models.PositiveIntegerField("购满件数", default=0)
    full_rate = models.FloatField('一件的优惠率,用于满N件优惠', default=0)
    full_price = models.DecimalField('金额,用于N件任买', max_digits=11, decimal_places=2, default=0)

    def __unicode__(self):
        return self.onsale_group_id


# 促销活动表
class OnsaleGroup(models.Model):
    verify_status_choices = [(0, '新建'), (1, '待审批'), (2, '已经上线'), (3, '不通过')]
    favor_type_choices = [(0, '满额减'), (1, '满额折'), (2, '满额换'), (3, '满件换'), (4, '满N件优惠'), (5, 'N件任买'), ]
    label_type_choices = [(0, '单商品标签'), (1, '多商品标签')]

    slug = models.CharField("slug", max_length=20, db_index=True, unique=True)
    name = models.CharField("促销活动名称", max_length=100)
    title = models.CharField("促销活动文案", max_length=100)
    comment = models.CharField("促销活动备注", max_length=100)

    favor_type = models.PositiveSmallIntegerField("优惠类型", default=0, choices=favor_type_choices)
    label_type = models.PositiveSmallIntegerField('商品标签类型', default=0, choices=label_type_choices)
    ware_label_id = models.PositiveIntegerField("商品标签ID")

    involve_count = models.PositiveIntegerField('参与次数', default=0)
    start_time = models.DateTimeField('活动开始时间', default=None, null=True, blank=True)
    end_time = models.DateTimeField('活动结束时间', default=None, null=True, blank=True)
    create_time = models.DateTimeField('创建时间', auto_now_add=True)
    update_time = models.DateTimeField('更新时间', default=None, null=True, blank=True)

    verify_status = models.PositiveSmallIntegerField('审批状态', default=0, choices=verify_status_choices)
    applicant = models.CharField("申请人", max_length=20)
    approver = models.CharField("审批人", max_length=20)
    modifier = models.CharField("修改人", max_length=20)

    jump_label = models.CharField('跳转类型', max_length=100, blank=True)
    jump_data = models.CharField('跳转数据', max_length=200, blank=True)

    def __unicode__(self):
        return self.name
