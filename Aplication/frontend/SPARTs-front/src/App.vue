<script setup>
import { Button, Column, InputText, ToggleSwitch, Select, SelectButton, Panel } from 'primevue';
import { ref } from 'vue';
const connected = ref (true);
const dialogSearch = ref (false);
const changeStoreItem = ref (false);
const newStorageItem = ref (null);
const readBucket = ref ({ x: null, y: null });

const filters = ref({
  global: { value: null, matchMode: "contains" },
  name: { value: null, matchMode: "contains" },
});

// Exemplo de dados
const products = ref([
  { name: "Laptop", image: "bamboo-watch.jpg", quantity: 5 },
  { name: "Keyboard", image: "black-watch.jpg", quantity: 8 },
  { name: "Mouse", image: "blue-t-shirt.jpg", quantity: 3 },
]);

</script>

<template>
  <div>
    <Panel class="mb-3">
      <div class="grid justify-content-center">
          <Button class="col-8" label="Connect to SPARTs" v-model:disabled="connected" />
      </div>
      <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Search item" @click="dialogSearch = true" :disabled="!connected"/>
      </div>
      <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Remap" :disabled="!connected"/>
      </div>
      <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Automatic Storage" :disabled="!connected"/>
      </div>
      <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Reorganize" :disabled="!connected"/>
      </div>
    </Panel>
    <Panel header="Store Bin" class="mb-3">
      <div class="grid justify-content-center align-items-center mt-3">
        <div class="flex flex-column align-items-center mt-3 col-2">
          <span class="mb-2">Change Storage Item ?</span>
          <ToggleSwitch
            v-model="changeStoreItem"
          />
        </div>

        <Select
          class="col-2 mr-3"
          v-model="newStorageItem"
          :disabled="!changeStoreItem"
          :options="products"
          size="small"
          placeholder="Select New Item To This Bin"
        />

        <Button
          class="col-4"
          label="Store bin"
          :disabled="!connected"
        />
      </div>
    </Panel>
    <Panel header="Read Bucket" class="mb-3">
      <div  class="grid justify-content-center align-items-center mt-3">
        <InputText
          class="col-1 mr-3"
          v-model="readBucket.x"
          placeholder="X Position"
          :disabled="!connected"
        />
        <InputText
          class="col-1 mr-3"
          v-model="readBucket.y"
          placeholder="Y Position"
          :disabled="!connected"
        />
        <Button
          class="col-1"
          label="Read Bucket"
          :disabled="!connected"
        />
        
      </div>
      <div class="grid justify-content-center align-items-center mt-3">
        <Span>Bucket Content: </Span>
      </div>
    </Panel>

    <Dialog v-model:visible="dialogSearch" modal header="Search Item" :style="{ width: '50rem' }">
      <div>
        <DataTable :value="products" :filters="filters" filterDisplay="menu" tableStyle="min-width: 50rem">

          <template #header>
          <div class="flex flex-wrap items-center justify-between gap-2">
            <span class="text-xl font-bold">Items</span>
            <span class="p-input-icon-left">
              <InputText placeholder="Search by name" v-model="filters.name.value"/>
            </span>
          </div>
          </template>

          <Column header="Action">
            <template #body="slotProps">
              <Button icon="pi pi-search" class="p-button-rounded" label="Fetch item" />
            </template>
          </Column>

          <Column field="name" header="Name"></Column>

          <Column header="Image">
            <template #body="slotProps">
              <img :src="`https://primefaces.org/cdn/primevue/images/product/${slotProps.data.image}`" :alt="slotProps.data.image" class="w-24 rounded" />
            </template>
          </Column>

          <Column field="quantity" header="Quantity"></Column>

        </DataTable>
      </div>
    </Dialog>
  </div>
</template>

<style scoped></style>
